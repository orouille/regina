
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Python Interface                                                      *
 *                                                                        *
 *  Copyright (c) 1999-2006, Ben Burton                                   *
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

#include "foreign/snappea.h"
#include "triangulation/ntriangulation.h"
#include <boost/python.hpp>

using namespace boost::python;

namespace {
    std::string (*stringToToken_chars)(const char*) = &regina::stringToToken;
    std::string (*stringToToken_string)(const std::string&) =
        &regina::stringToToken;
}

void addForeignSnapPea() {
    def("readSnapPea", regina::readSnapPea,
        return_value_policy<manage_new_object>());
    def("writeSnapPea", regina::writeSnapPea);
    def("stringToToken", stringToToken_chars);
    def("stringToToken", stringToToken_string);
}

