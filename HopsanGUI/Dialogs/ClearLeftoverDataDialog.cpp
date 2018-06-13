/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

#include "ClearLeftoverDataDialog.h"
#include "global.h"
#include "Configuration.h"

#include <QPushButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

ClearLeftoverDataDialog::ClearLeftoverDataDialog(QWidget *pParent) : QDialog(pParent)
{
    auto pText = new QLabel(this);
    pText->setText(
R"(There are files present in the LogCache directory!

They may be used by an other instance of Hopsan or be leftover
from an abnormal program termination

(This message will automatically close after 10 seconds!))");

    auto pQuestion = new QLabel(this);
    pQuestion->setText("Do you want to clear them?");

    auto pYesButton = new QPushButton("Yes", this);
    connect(pYesButton, SIGNAL(clicked(bool)), this, SLOT(accept()));

    auto pNoButton = new QPushButton("No", this);
    pNoButton->setDefault(true);
    connect(pNoButton, SIGNAL(clicked(bool)), this, SLOT(reject()));

    auto pAlwaysClear = new QCheckBox("Always clear, do not ask!", this);
    pAlwaysClear->setChecked(gpConfig->getBoolSetting(CFG_ALLWAYSCLEARLEFTOVERS));
    connect(pAlwaysClear, SIGNAL(clicked(bool)), this, SLOT(toggleAlwaysClear(bool)));

    auto pBottomHLayout = new QHBoxLayout();
    pBottomHLayout->addWidget(pAlwaysClear, 2);
    pBottomHLayout->addWidget(pYesButton, 1);
    pBottomHLayout->addWidget(pNoButton, 1);

    auto pLayout = new QVBoxLayout(this);
    pLayout->addWidget(pText);
    pLayout->addWidget(pQuestion);
    pLayout->addLayout(pBottomHLayout);
}

void ClearLeftoverDataDialog::toggleAlwaysClear(bool tf)
{
    gpConfig->setBoolSetting(CFG_ALLWAYSCLEARLEFTOVERS, tf);
}


