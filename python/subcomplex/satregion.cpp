
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Python Interface                                                      *
 *                                                                        *
 *  Copyright (c) 1999-2018, Ben Burton                                   *
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

#include "../pybind11/pybind11.h"
#include "manifold/sfs.h"
#include "subcomplex/satregion.h"
#include <iostream>
#include "../helpers.h"

using regina::SatBlock;
using regina::SatBlockSpec;
using regina::SatRegion;

namespace {
}

void addSatRegion(pybind11::module& m) {
    auto s = pybind11::class_<SatBlockSpec>(m, "SatBlockSpec")
        .def(pybind11::init<>())
        .def(pybind11::init<SatBlock*, bool, bool>())
        .def(pybind11::init<const SatBlockSpec&>())
        .def_readonly("block", &SatBlockSpec::block)
        .def_readonly("refVert", &SatBlockSpec::refVert)
        .def_readonly("refHoriz", &SatBlockSpec::refHoriz)
    ;
    regina::python::add_eq_operators(s);

    m.attr("NSatBlockSpec") = m.attr("SatBlockSpec");

    auto r = pybind11::class_<SatRegion>(m, "SatRegion")
        .def(pybind11::init<SatBlock*>())
        .def("numberOfBlocks", &SatRegion::numberOfBlocks)
        .def("block", &SatRegion::block,
            pybind11::return_value_policy::reference_internal)
        .def("blockIndex", &SatRegion::blockIndex)
        .def("numberOfBoundaryAnnuli", &SatRegion::numberOfBoundaryAnnuli)
        .def("boundaryAnnulus", [](const SatRegion& r, unsigned which) {
            SatBlock* block;
            unsigned annulus;
            bool blockRefVert, blockRefHoriz;

            r.boundaryAnnulus(which, block, annulus,
                blockRefVert, blockRefHoriz);
            return pybind11::make_tuple(
                block, annulus, blockRefVert, blockRefHoriz);
        }, pybind11::return_value_policy::reference_internal)
        .def("createSFS", &SatRegion::createSFS)
        .def("expand", [](SatRegion& r, bool stopIfIncomplete) {
            SatBlock::TetList avoidTets;
            return r.expand(avoidTets, stopIfIncomplete);
        }, pybind11::arg("stopIfIncomplete") = false)
        .def("writeBlockAbbrs", [](const SatRegion& r, bool tex) {
            r.writeBlockAbbrs(std::cout, tex);
        }, pybind11::arg("tex") = false)
        .def("writeDetail", [](const SatRegion& r, const std::string& title) {
            r.writeDetail(std::cout, title);
        })
    ;
    regina::python::add_output(r);
    regina::python::add_eq_operators(r);

    m.attr("NSatRegion") = m.attr("SatRegion");

}

