
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Computational Engine                                                  *
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

/*! \file nsatregion.h
 *  \brief Supports connected regions of saturated blocks in triangulations
 *  of Seifert fibred spaces.
 */

#ifndef __NSATREGION_H
#ifndef __DOXYGEN
#define __NSATREGION_H
#endif

#include "subcomplex/nsatblock.h"
#include <vector>

namespace regina {

/**
 * \weakgroup subcomplex
 * @{
 */

/**
 * Describes how a single saturated block forms a part of a larger
 * saturated region.
 *
 * A saturated region consists of several saturated blocks joined
 * together along their boundary annuli.  This is a helper structure
 * containing a single saturated block along with details of its
 * orientation within a larger region.
 *
 * The ring of saturated annuli around the boundary of the block gives a
 * natural orientation to the block within the context of the base orbifold,
 * just as the ring of edges around a polygon would give a natural
 * orientation to that polygon within the context of a surrounding surface.
 * Again drawing an analogy with the orientation of polygons within a surface,
 * each block can be considered to have a correct or reflected orientation
 * according to whether this ring of annuli runs clockwise or anticlockwise
 * in the base orbifold.
 *
 * The precise orientation of a block is described using two booleans.
 * A block may be reflected \e horizontally, which preserves the
 * directions of Seifert fibres but which reverses the
 * clockwise/anticlockwise orientation as discussed above.  A block may
 * also be reflected \e vertically, which preserves the
 * clockwise/anticlockwise orientation but which reverses the directions
 * of the Seifert fibres.  A block may of course be reflected both
 * horizontally and vertically, or it may not be reflected at all.
 *
 * This helper structure is small, and can be copied by value if
 * necessary.  Be aware that when this structure is destroyed, the
 * internal block structure of type NSatBlock is \e not destroyed.
 */
struct NSatBlockSpec {
    NSatBlock* block;
        /**< Details of the saturated block structure. */
    bool refVert;
        /**< Indicates whether the block is reflected vertically within
             the larger region.  See the class notes for details. */
    bool refHoriz;
        /**< Indicates whether the block is reflected horizontally within
             the larger region.  See the class notes for details. */

    /**
     * Creates a new structure that is completely uninitialised.
     */
    NSatBlockSpec();
    /**
     * Creates a new structure that is initialised to the given set of
     * values.
     *
     * @param useBlock details of the saturated block structure.
     * @param useRefVert \c true if the block is reflected vertically
     * within the larger region, or \c false otherwise.
     * @param useRefHoriz \c true if the block is reflected horizontally
     * within the larger region, or \c false otherwise.
     */
    NSatBlockSpec(NSatBlock* useBlock, bool useRefVert, bool useRefHoriz);
};

/**
 * A large saturated region in a Seifert fibred space formed by joining
 * together saturated blocks.
 *
 * Like a saturated block (described in the class NSatBlock), a
 * saturated region is a connected set of tetrahedra built from a subset
 * of fibres.  Unlike a saturated block however, a saturated region has
 * no constraints on its boundary - it may have several boundary
 * components or it may have none.  For instance, a saturated region
 * might be an entire closed Seifert fibred space, or it might describe
 * a Seifert fibred component of a JSJ decomposition.
 *
 * A saturated region is formed from a collection of saturated blocks by
 * joining the boundary annuli of these blocks together in pairs.  The
 * joins must be made so that the fibres are consistent, though it is
 * allowable to reverse the directions of the fibres.  There is no problem
 * with joining two boundary annuli from the same block to each other.
 *
 * Any boundary annulus of a block that is not joined to some other
 * boundary annulus of a block becomes a boundary annulus of the entire
 * region.  In this way, each boundary component of the region (if there
 * are any at all) is formed from a ring of boundary annuli, in the same
 * way that the boundary of a block is.  Note that the routine
 * NSatBlock::nextBoundaryAnnulus() can be used to trace around a region
 * boundary.  Like block boundaries, the boundary of a saturated region
 * need not be part of the boundary of the larger triangulation (i.e.,
 * there may be adjacent tetrahedra that are not recognised as part of
 * this saturated structure).
 *
 * The NSatRegion class stores a list of its constituent blocks, but it
 * does not directly store which block boundary annuli are joined to
 * which.  This adjacency information is stored within the blocks
 * themselves; see the notes regarding adjacency in the NSatBlock class
 * description.
 *
 * Blocks cannot be added to a region by hand.  The way a region is
 * constructed is by locating some initial block within a triangulation
 * and passing this to the NSatRegion constructor, and then by calling
 * expand() to locate adjacent blocks and expand the region as far as
 * possible.  For locating initial blocks, the class
 * NSatBlockStarterSearcher may be of use.
 *
 * \warning It is crucial that the adjacency information stored in the
 * blocks is consistent with the region containing them.  All this
 * requires is that the blocks are not manipulated externally (e.g.,
 * NSatBlock::setAdjacent() is not called on any of the blocks), but
 * instead all adjacency information is managed by this class.  Routines
 * such as expand() which may add more blocks to the region will update
 * the block adjacencies accordingly.
 *
 * \todo Have this class track the boundary components properly, with
 * annuli grouped and oriented according to the region boundaries (as
 * opposed to individual block boundaries).
 */
class NSatRegion : public ShareableObject {
    private:
        typedef std::vector<NSatBlockSpec> BlockSet;
            /**< The data structure used to store the list of
                 constituent blocks. */

        BlockSet blocks_;
            /**< The set of blocks from which this region is formed,
                 along with details of how they are oriented within this
                 larger region. */
        long baseEuler_;
            /**< The Euler characteristic of the base orbifold if we
                 assume that each block contributes a trivial disc to
                 the base orbifold. */
        bool baseOrbl_;
            /**< Denotes whether the base orbifold is orientable if we
                 assume that each block contributes a trivial disc to
                 the base orbifold. */
        bool hasTwist_;
            /**< Denotes whether we can find a fibre-reversing path that does
                 not step inside the interior of any constituent blocks. */
        bool twistsMatchOrientation_;
            /**< Denotes whether set of fibre-reversing paths corresponds
                 precisely to the set of orientation-reversing paths on the
                 base orbifold, where we do not allow paths that step inside
                 the interior of any constituent blocks. */
        long shiftedAnnuli_;
            /**< The number of additional (1,1) twists added to the underlying
                 Seifert fibred space due to blocks being sheared up or down
                 as they are joined together.  This typically happens when
                 the triangulations of two boundary annuli are not compatible
                 when joined (e.g., they provide opposite diagonal edges) */
        unsigned long extraReflectors_;
            /**< The number of constituent blocks with twisted boundary.
                 Each such block provides a new reflector boundary to the
                 base orbifold; see NSatBlock::adjustSFS() for details. */

        unsigned long nBdryAnnuli_;
            /**< The number of saturated annuli forming the boundary
                 components (if any) of this region. */

    public:
        /**
         * Constructs a new region containing just the given block.
         * All boundary annuli of the given block will become boundary
         * annuli of this region.  It is guaranteed that this block will
         * be stored in the region without any kind of reflection (see
         * NSatBlockSpec for details).
         *
         * Typically a region is initialised using this constructor, and
         * then grown using the expand() routine.  For help in finding
         * an initial starter block, see the NSatBlockStarterSearcher class.
         *
         * This region will claim ownership of the given block, and upon
         * destruction it will destroy this block also.
         *
         * \pre The given block has no adjacencies listed.  That is,
         * for every boundary annulus of the given block,
         * NSatBlock::hasAdjacentBlock() returns \c false.
         *
         * @param starter the single block that this region will describe.
         */
        NSatRegion(NSatBlock* starter);

        /**
         * Destroys this structure and all of its internal data,
         * including the individual blocks that make up this region.
         */
        virtual ~NSatRegion();

        /**
         * Returns the number of saturated annuli that together form the
         * boundary components of this region.
         *
         * @return the number of boundary annuli.
         */
        unsigned long numberOfBoundaryAnnuli() const;
        /**
         * Returns the requested saturated annulus on the boundary of
         * this region.
         *
         * The saturated annuli that together form the boundary
         * components of this region are numbered from 0 to
         * numberOfBoundaryAnnuli()-1 inclusive.  The argument \a which
         * specifies which one of these annuli should be returned.
         *
         * Currently the annuli are numbered lexicographically by
         * block and then by annulus number within the block, although
         * this ordering is subject to change in future versions of Regina.
         * In particular, the annuli are \e not necessarily numbered in
         * order around the region boundaries, and each region boundary
         * component might not even be given a consecutive range of numbers.
         *
         * It is guaranteed however that, if the starter block passed to
         * the NSatRegion constructor provides any boundary annuli for
         * the overall region, then the first such annulus in the starter
         * block will be numbered 0 here.
         *
         * The structure returned will be the annulus precisely as it
         * appears within its particular saturated block.  As discussed
         * in the NSatBlockSpec class notes, the block might be
         * reflected horizontally and/or vertically within the overall
         * region, which will affect how the annulus is positioned as
         * part of the overall region boundary (e.g., the annulus might
         * be positioned upside-down in the overall region boundary,
         * or it might be positioned with its second face appearing before
         * its first face as one walks around the boundary).  To account
         * for this, the two boolean arguments \a blockRefVert and
         * \a blockRefHoriz will be modified to indicate if and how the
         * block is reflected.
         *
         * \warning This routine is quite slow, since it currently scans
         * through every annulus of every saturated block.  Use it
         * sparingly!
         *
         * @param which specifies which boundary annulus of this region
         * to return; this must be between 0 and numberOfBoundaryAnnuli()-1
         * inclusive.
         * @param blockRefVert used to return whether the block
         * containing the requested annulus is vertically reflected
         * within this region (see NSatBlockSpec for details).  This
         * will be set to \c true if the block is vertically reflected,
         * or \c false if not.
         * @param blockRefHoriz used to return whether the block
         * containing the requested annulus is horizontally reflected
         * within this region (see NSatBlockSpec for details).  This
         * will be set to \c true if the block is horizontally reflected,
         * or \c false if not.
         * @return details of the requested boundary annulus, precisely
         * as it appears within its particular saturated block.
         */
        const NSatAnnulus& boundaryAnnulus(unsigned long which,
            bool& blockRefVert, bool& blockRefHoriz) const;
        /**
         * TODO
         */
        void boundaryAnnulus(unsigned long which,
            NSatBlock*& block, unsigned& annulus,
            bool& blockRefVert, bool& blockRefHoriz) const;

        /**
         * Returns details of the Seifert fibred space represented by
         * this region.
         *
         * In order to correctly construct the Seifert fibred space,
         * this routine must know how many boundary components this
         * region has (as each boundary component will become a puncture
         * in the base orbifold).  Since the NSatRegion class does not
         * yet have sophisticated boundary tracking, the caller of this
         * routine must provide this information in the argument
         * \a nBoundaries.  Be aware that this argument may be removed
         * in future versions of Regina.
         *
         * Each boundary component will be formed from a ring of
         * saturated annuli, which together form a torus (note that
         * Klein bottles are disallowed here; see the preconditions below).
         * The NSatBlock class notes describe how a curve representing the
         * base orbifold runs through each saturated annulus about a single
         * block; the curves representing the base orbifold on the
         * overall region boundaries are formed by piecing together
         * the relevant pieces of such curves on individual blocks.
         * In other words, the curves on the region boundaries that
         * represent the base orbifold cut through each annulus parallel
         * to the edges joining markings 0 and 2 (see the NSatAnnulus
         * notes for a discussion of markings).
         *
         * If the argument \a reflect is \c true, the Seifert fibred
         * space will be created as though the entire region had been
         * reflected.  In particular, each twist or exceptional fibre
         * will be negated before being added to the Seifert structure.
         *
         * \pre The argument \a nBoundaries is indeed the number of
         * boundary components of this region (or equivalently, the
         * number of punctures in the base orbifold).
         * \pre None of the boundary components of this region are
         * twisted loops of saturated annuli.  In other words, each
         * boundary component forms a torus, not a Klein bottle.
         *
         * @param nBoundaries the number of boundary components of this
         * saturated region.
         * @param reflect \c true if this region is to be reflected
         * as the Seifert fibred space is created, or \c false if not.
         * @return the newly created structure of the underlying Seifert
         * fibred space.
         */
        NSFSpace* createSFS(long nBoundaries, bool reflect) const;

        /**
         * Expands this region as far as possible within the overall
         * triangulation.  This routine will hunt for new saturated
         * blocks, and will also hunt for new adjacencies between
         * existing blocks.
         *
         *
         * 
         *
         * TODO: Document expand().
         *
         * stopIfBounded: true means we stop expanding as soon as we
         * find a boundary annulus that has some adjacent tetrahedron
         * (even if just on one face) but no corresponding adjacent block.
         * When we stop the structure will be in an inconsistent state;
         * it is assumed that it will be tossed away completely.
         *
         * Guarantee that new blocks will be pushed to the end of the
         * list (i.e., their indices won't change).
         *
         * \warning When joining blocks together, it is possible to
         * create invalid edges (e.g., by joining a one-annulus
         * untwisted boundary to a one-annulus twisted boundary).
         * This routine does \e not check for such conditions.  It is
         * recommended that you run NTriangulation::isValid() before
         * calling this routine.
         *
         * \pre TODO Any block adjacencies are in this list.
         *
         * Returns false iff we passed stopIfBounded but it failed.
         */
        bool expand(NSatBlock::TetList& avoidTets, bool stopIfBounded = false);

        void writeTextShort(std::ostream& out) const;

    private:
        /**
         * Runs through the region structure and recalculates the
         * \a baseEuler_ data member.
         *
         * No assumptions are made about whether edges of the boundary
         * annuli become identified due to features outside the region.
         * That is, this routine is safe to call even when this region
         * is joined to some other not-yet-understood sections of the
         * triangulation.
         */
        void calculateBaseEuler();
};

/*@}*/

// Inline functions for NSatBlockSpec

inline NSatBlockSpec::NSatBlockSpec() {
}

inline NSatBlockSpec::NSatBlockSpec(NSatBlock* useBlock, bool useRefVert,
        bool useRefHoriz) : block(useBlock), refVert(useRefVert),
        refHoriz(useRefHoriz) {
}

// Inline functions for NSatRegion

inline unsigned long NSatRegion::numberOfBoundaryAnnuli() const {
    return nBdryAnnuli_;
}

} // namespace regina

#endif

