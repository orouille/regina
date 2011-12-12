
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  KDE User Interface                                                    *
 *                                                                        *
 *  Copyright (c) 1999-2011, Ben Burton                                   *
 *  For further details contact Ben Burton (bab@debian.org).              *
 *                                                                        *
 *  This program is free software; you can redistribute it and/or         *
 *  modify it under the terms of the GNU General Public License as        *
 *  published by the Free Software Foundation; either version 2 of the    *
 *  License, or (at your option) any later version.                       *
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

/*! \file reginafilter.h
 *  \brief A variety of filename filters for use with KFileDialog.
 */

#ifndef __REGINAFILTER_H
#define __REGINAFILTER_H

/**
 * Filename filter for all supported files.
 */
#define FILTER_SUPPORTED \
    "Regina Data Files (*.rga);;All Files (*)"

/**
 * Filename filter for Regina data files.
 */
#define FILTER_REGINA \
    "Regina Data Files (*.rga);;All Files (*)"

/**
 * Filename filter for Python libraries.
 */
#define FILTER_PYTHON_LIBRARIES \
    "Python Libraries (*.py);;All Files (*)"

/**
 * Filename filter for PDF documents.
 */
#define FILTER_PDF \
    "PDF Documents (*.pdf);;All Files (*)"

/**
 * Filename filter for Python scripts.
 */
#define FILTER_PYTHON_SCRIPTS \
    "Python Scripts (*.py);;All Files (*)"

/**
 * Filename filter for SnapPea files.
 */
#define FILTER_SNAPPEA \
    "SnapPea Files (*.tri);;All Files (*)"

/**
 * Filename filter for Orb files.
 */
#define FILTER_ORB \
    "Orb and Casson Files (*.orb);;All Files (*)"

/**
 * Filename filter for C++ source files.
 */
#define FILTER_CPP_SOURCE \
    "C++ Source Files (*.cpp *.cc *.C);;All Files (*)"

/**
 * Filename filter for CSV (comma-separated value) files.
 */
#define FILTER_CSV \
    "Text Files with Comma-Separated Values (*.csv);;All Files (*)"

/**
 * Filename filter for all files.
 */
#define FILTER_ALL \
    "All Files (*)"

#endif

