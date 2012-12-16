/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (c) 2011, RetroShare Team
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

#ifndef _FILESDEFS_H
#define _FILESDEFS_H

#include <QString>
#include <QIcon>

class FilesDefs
{
public:
	static QString getImageFromFilename(const QString& filename, bool anyForUnknown);
	static QIcon getIconFromFilename(const QString& filename);
	static QString getNameFromFilename(const QString& filename);
};

#endif

