
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  KDE User Interface                                                    *
 *                                                                        *
 *  Copyright (c) 1999-2013, Ben Burton                                   *
 *  For further details contact Ben Burton (bab@debian.org).              *
 *                                                                        *
 *  This program is free software; you can redistribute it and/or         *
 *  modify it under the terms of the GNU General Public License as        *
 *  published by the Free Software Foundation; either version 2 of the    *
 *  License, or (at your option) any later version.                       *
 *                                                                        *
 *  As an exception, when this program is distributed through (i) the     *
 *  App Store by Apple Inc.; (ii) the Mac App Store by Apple Inc.; or     *
 *  (iii) Google Play by Google Inc., then that store may impose any      *
 *  digital rights management, device limits and/or redistribution        *
 *  restrictions that are required by its terms of service.               *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful, but   *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *  General Public License for more details.                              *
 *                                                                        *
 *  You should have received a copy of the GNU General Public             *
 *  License along with this program; if not, write to the Free            *
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,       *
 *  MA 02110-1301, USA.                                                   *
 *                                                                        *
 **************************************************************************/

/* end stub */

// Regina core includes:
#include "progress/nprogressmanager.h"
#include "progress/nprogresstypes.h"

// UI includes:
#include "progressdialogs.h"

#include <QApplication>
#include <QFrame>
#include <QLabel>
#include <QThread>
#include <QVBoxLayout>

// Indicates how we convert floating-point percentages to integer counts.
// Integer progress will range from 0 to SLICES.
// For the arithmetic to be correct, SLICES must be a multiple of 100.
#define SLICES 1000

namespace {
    /**
     * A subclass of QThread that gives us public access to sleep
     * routines.
     */
    class WaitingThread : public QThread {
        public:
            static void tinySleep() {
                QThread::usleep(250);
            }
    };
}

ProgressDialogNumeric::ProgressDialogNumeric(
        regina::NProgressManager* useManager,
        const QString& displayText, QWidget* parent) :
        QProgressDialog(parent),
        /* Don't use Qt::Popup because the layout breaks under fink. */
        manager(useManager), progress(0) {
    setLabelText(displayText);
    setWindowTitle(tr("Working"));
    setMinimumDuration(500);
    setWindowModality(Qt::WindowModal);
}

bool ProgressDialogNumeric::run() {
    show();
    QCoreApplication::instance()->processEvents();

    while (! manager->isStarted())
        WaitingThread::tinySleep();

    progress = manager->getProgress();
    setMinimum(0);
    setMaximum(SLICES);
    while (! progress->isFinished()) {
        if (wasCanceled())
            progress->cancel();
        if (progress->hasChanged())
            setValue(progress->getPercent() * (SLICES / 100));
        QCoreApplication::instance()->processEvents();
        WaitingThread::tinySleep();
    }

    return (! progress->isCancelled());
}

ProgressDialogMessage::ProgressDialogMessage(
        regina::NProgressManager* useManager,
        const QString& displayText, QWidget* parent) :
        QDialog(parent),
        manager(useManager), progress(0) {
    setWindowTitle(tr("Working"));
    setWindowModality(Qt::WindowModal);

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* label = new QLabel(QString("<qt><b>%1</b></qt>").arg(displayText));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    QFrame* separator = new QFrame();
    separator->setFrameStyle(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    layout->addWidget(separator);

    msg = new QLabel(tr("Status: Starting"));
    msg->setAlignment(Qt::AlignLeft);
    msg->setTextFormat(Qt::PlainText);
    layout->addWidget(msg);

    layout->addStretch(1);
}

bool ProgressDialogMessage::run() {
    show();
    QCoreApplication::instance()->processEvents();

    while (! manager->isStarted())
        WaitingThread::tinySleep();

    progress = manager->getProgress();
    msg->setText(tr("Status: %1").arg(progress->getDescription().c_str()));
    while (! progress->isFinished()) {
        if (progress->hasChanged()) {
            msg->setText(tr("Status: %1").arg(
                progress->getDescription().c_str()));
        }
        QCoreApplication::instance()->processEvents();
        WaitingThread::tinySleep();
    }

    return (! progress->isCancelled()); // Always true, for now.
}

