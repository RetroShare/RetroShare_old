/*
 * Retroshare Gxs Support
 *
 * Copyright 2012-2013 by Robert Fernie.
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

#ifndef _GXS_ID_DETAILS_H
#define _GXS_ID_DETAILS_H

#include <QIcon>
#include <QString>
#include <retroshare/rsidentity.h>
#include <list>

namespace GxsIdDetails
{

	bool MakeIdDesc(const RsGxsId &id, bool doIcons, 
			QString &desc, std::list<QIcon> &icons,QString& comment);

	bool GenerateCombinedIcon(QIcon &outIcon, std::list<QIcon> &icons);

    QImage makeDefaultIcon(const RsGxsId& id);

} // namespace GxsIdDetails.

#endif
