/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2008 RetroShare Team
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

#include <QHBoxLayout>
#include <QLabel>

#include "ratesstatus.h"
#include <retroshare/rsiface.h>

#include <iomanip>

RatesStatus::RatesStatus(QWidget *parent)
 : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setMargin(0);
    hbox->setSpacing(6);
    
    iconLabel = new QLabel( this );
    iconLabel->setPixmap(QPixmap(":/images/up0down0.png"));
    // iconLabel doesn't change over time, so we didn't need a minimum size
    hbox->addWidget(iconLabel);
    
    statusRates = new QLabel( tr("<strong>Down:</strong> 0.00 (kB/s) | <strong>Up:</strong> 0.00 (kB/s) "), this );
//    statusPeers->setMinimumSize( statusPeers->frameSize().width() + 0, 0 );
    hbox->addWidget(statusRates);
    
    setLayout(hbox);
}

void RatesStatus::getRatesStatus(float downKb, float upKb)
{
    /* set users/friends/network */

    if (statusRates)
          statusRates -> setText(QString("<strong>%1:</strong> %2 (kB/s) | <strong>%3:</strong> %4 (kB/s) ").arg(tr("Down")).arg(downKb, 0, 'f', 2).arg(tr("Up")).arg(upKb, 0, 'f', 2));

    if( upKb > 0 || downKb < 0  )
    {
        iconLabel->setPixmap(QPixmap(":/images/up1down0.png"));
    }
    
    if( upKb < 0 || downKb > 0 )
    {
        iconLabel->setPixmap(QPixmap(":/images/up0down1.png"));
    }
    
    if( upKb > 0 || downKb > 0 )
    {
        iconLabel->setPixmap(QPixmap(":/images/up1down1.png"));
    }
        
    else
    {
        iconLabel->setPixmap(QPixmap(":/images/up0down0.png"));
    }
}
