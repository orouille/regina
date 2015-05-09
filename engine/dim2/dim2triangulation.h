
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Computational Engine                                                  *
 *                                                                        *
 *  Copyright (c) 1999-2014, Ben Burton                                   *
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

/*! \file dim2/dim2triangulation.h
 *  \brief Deals with 2-dimensional triangulations.
 */

#ifndef __DIM2TRIANGULATION_H
#ifndef __DOXYGEN
#define __DIM2TRIANGULATION_H
#endif

#include <memory>
#include <vector>
#include "regina-core.h"
#include "generic/ngenerictriangulation.h"
#include "generic/triangulation.h"
#include "packet/npacket.h"
#include "utilities/nmarkedvector.h"
#include "utilities/nproperty.h"

// The following headers are necessary so that std::auto_ptr can invoke
// destructors where necessary.
#include "dim2/dim2isomorphism.h"

// NOTE: More #includes follow after the class declarations.

namespace regina {

class Dim2BoundaryComponent;
class Dim2Edge;
class Dim2Vertex;
class NXMLDim2TriangulationReader;
class NXMLPacketReader;

template <int> class Component;
template <int> class Isomorphism;
template <int> class SimplexBase;
template <int> class Simplex;
typedef Isomorphism<2> Dim2Isomorphism;
typedef Simplex<2> Dim2Triangle;

/**
 * \addtogroup dim2 2-Manifold Triangulations
 * Triangulations of 2-manifolds.
 * @{
 */

#ifndef __DOXYGEN // Doxygen complains about undocumented specialisations.
template <>
struct PacketInfo<PACKET_DIM2TRIANGULATION> {
    typedef Triangulation<2> Class;
    inline static const char* name() {
        return "2-Manifold Triangulation";
    }
};
#endif

/**
 * Represents a 2-manifold triangulation.
 *
 * This is a specialisation of the generic Triangulation class template;
 * see the Triangulation documentation for a general overview of how
 * the triangulation classes work.
 *
 * This 2-dimensional specialisation offers significant extra functionality,
 * including many functions specific to 2-manifolds, plus rich details of
 * the combinatorial structure of the triangulation.
 *
 * In particular, this class also tracks the vertices and edges of the
 * triangulation (as represented by the classes Dim2Vertex and Dim2Edge),
 * as well as boundary components (as represented by the class
 * Dim2BoundaryComponent).  Such objects are temporary: whenever the
 * triangulation changes, these objects will be deleted and rebuilt, and so
 * any pointers to them will become invalid.  Likewise, if the triangulation
 * is deleted then these objects will be deleted alongside it.
 */
template <>
class REGINA_API Triangulation<2> :
        public NPacket,
        public TriangulationBase<2>,
        public NGenericTriangulation<2> {
    REGINA_PACKET(Triangulation<2>, PACKET_DIM2TRIANGULATION)

    public:
        typedef std::vector<Dim2Triangle*>::const_iterator TriangleIterator;
            /**< A dimension-specific alias for SimplexIterator,
                 used to iterate through triangles. */
        typedef std::vector<Dim2Edge*>::const_iterator EdgeIterator;
            /**< Used to iterate through edges. */
        typedef std::vector<Dim2Vertex*>::const_iterator VertexIterator;
            /**< Used to iterate through vertices. */
        typedef std::vector<Dim2BoundaryComponent*>::const_iterator
                BoundaryComponentIterator;
            /**< Used to iterate through boundary components. */

    private:
        mutable NMarkedVector<Dim2Edge> edges_;
            /**< The edges in the triangulation skeleton. */
        mutable NMarkedVector<Dim2Vertex> vertices_;
            /**< The vertices in the triangulation skeleton. */
        mutable NMarkedVector<Dim2BoundaryComponent> boundaryComponents_;
            /**< The components that form the boundary of the
                 triangulation. */

    public:
        /**
         * \name Constructors and Destructors
         */
        /*@{*/

        /**
         * Default constructor.
         *
         * Creates an empty triangulation.
         */
        Triangulation();
        /**
         * Creates a new copy of the given triangulation.
         * The packet tree structure and packet label are \e not copied.
         *
         * @param copy the triangulation to copy.
         */
        Triangulation(const Triangulation& copy);
        /**
         * "Magic" constructor that tries to find some way to interpret
         * the given string as a triangulation.
         *
         * At present, Regina understands the following types of strings
         * (and attempts to parse them in the following order):
         *
         * - isomorphism signatures (see fromIsoSig()).
         *
         * This list may grow in future versions of Regina.
         *
         * Regina will also set the packet label accordingly.
         *
         * If Regina cannot interpret the given string, this will be
         * left as the empty triangulation.
         *
         * @param description a string that describes a 2-manifold
         * triangulation.
         */
        Triangulation(const std::string& description);
        /**
         * Destroys this triangulation.
         *
         * The constituent triangles, the cellular structure and all other
         * properties will also be destroyed.
         */
        virtual ~Triangulation();

        /*@}*/
        /**
         * \name Packet Administration
         */
        /*@{*/

        virtual void writeTextShort(std::ostream& out) const;
        virtual void writeTextLong(std::ostream& out) const;
        virtual bool dependsOnParent() const;

        /*@}*/
        /**
         * \name Triangles
         */
        /*@{*/

        /**
         * A dimension-specific alias for size().
         *
         * See size() for further information.
         */
        unsigned long getNumberOfTriangles() const;
        /**
         * A dimension-specific alias for simplices().
         *
         * See simplices() for further information.
         */
        const std::vector<Dim2Triangle*>& getTriangles() const;
        /**
         * A dimension-specific alias for simplex().
         *
         * See simplex() for further information.
         */
        Dim2Triangle* getTriangle(unsigned long index);
        /**
         * A dimension-specific alias for simplex().
         *
         * See simplex() for further information.
         */
        const Dim2Triangle* getTriangle(unsigned long index) const;
        /**
         * A dimension-specific alias for simplexIndex().
         *
         * See simplexIndex() for further information.
         */
        long triangleIndex(const Dim2Triangle* tri) const;
        /**
         * A dimension-specific alias for newSimplex().
         *
         * See newSimplex() for further information.
         */
        Dim2Triangle* newTriangle();
        /**
         * A dimension-specific alias for newSimplex().
         *
         * See newSimplex() for further information.
         */
        Dim2Triangle* newTriangle(const std::string& desc);
        /**
         * A dimension-specific alias for removeSimplex().
         *
         * See removeSimplex() for further information.
         */
        void removeTriangle(Dim2Triangle* tri);
        /**
         * A dimension-specific alias for removeSimplexAt().
         *
         * See removeSimplexAt() for further information.
         */
        void removeTriangleAt(unsigned long index);
        /**
         * A dimension-specific alias for removeAllSimplices().
         *
         * See removeAllSimplices() for further information.
         */
        void removeAllTriangles();

        /*@}*/
        /**
         * \name Skeletal Queries
         */
        /*@{*/

        /**
         * Returns the number of boundary components in this triangulation.
         *
         * @return the number of boundary components.
         */
        unsigned long getNumberOfBoundaryComponents() const;
        /**
         * Returns the number of vertices in this triangulation.
         *
         * @return the number of vertices.
         */
        unsigned long getNumberOfVertices() const;
        /**
         * Returns the number of edges in this triangulation.
         *
         * @return the number of edges.
         */
        unsigned long getNumberOfEdges() const;
        /**
         * Returns the number of faces of the given dimension in this
         * triangulation.
         *
         * This template function is to assist with writing dimension-agnostic
         * code that can be reused to work in different dimensions.
         *
         * \pre The template argument \a subdim is between 0 and 2 inclusive.
         *
         * \ifacespython Not present.
         *
         * @return the number of faces of the given dimension.
         */
        template <int subdim>
        unsigned long getNumberOfFaces() const;

        /**
         * Returns all boundary components of this triangulation.
         *
         * Bear in mind that each time the triangulation changes, the
         * boundary components will be deleted and replaced with new
         * ones.  Thus the objects contained in this list should be
         * considered temporary only.
         *
         * This reference to the list however will remain valid and
         * up-to-date for as long as the triangulation exists.
         *
         * \ifacespython This routine returns a python list.
         *
         * @return the list of all boundary components.
         */
        const std::vector<Dim2BoundaryComponent*>& getBoundaryComponents()
            const;
        /**
         * Returns all vertices of this triangulation.
         *
         * Bear in mind that each time the triangulation changes, the
         * vertices will be deleted and replaced with new
         * ones.  Thus the objects contained in this list should be
         * considered temporary only.
         *
         * This reference to the list however will remain valid and
         * up-to-date for as long as the triangulation exists.
         *
         * \ifacespython This routine returns a python list.
         *
         * @return the list of all vertices.
         */
        const std::vector<Dim2Vertex*>& getVertices() const;
        /**
         * Returns all edges of this triangulation.
         *
         * Bear in mind that each time the triangulation changes, the
         * edges will be deleted and replaced with new
         * ones.  Thus the objects contained in this list should be
         * considered temporary only.
         *
         * This reference to the list however will remain valid and
         * up-to-date for as long as the triangulation exists.
         *
         * \ifacespython This routine returns a python list.
         *
         * @return the list of all edges.
         */
        const std::vector<Dim2Edge*>& getEdges() const;
        /**
         * Returns the requested triangulation boundary component.
         *
         * Bear in mind that each time the triangulation changes, the
         * boundary components will be deleted and replaced with new
         * ones.  Thus this object should be considered temporary only.
         *
         * @param index the index of the desired boundary component, ranging
         * from 0 to getNumberOfBoundaryComponents()-1 inclusive.
         * @return the requested boundary component.
         */
        Dim2BoundaryComponent* getBoundaryComponent(unsigned long index) const;
        /**
         * Returns the requested triangulation vertex.
         *
         * Bear in mind that each time the triangulation changes, the
         * vertices will be deleted and replaced with new
         * ones.  Thus this object should be considered temporary only.
         *
         * @param index the index of the desired vertex, ranging from 0
         * to getNumberOfVertices()-1 inclusive.
         * @return the requested vertex.
         */
        Dim2Vertex* getVertex(unsigned long index) const;
        /**
         * Returns the requested triangulation edge.
         *
         * Bear in mind that each time the triangulation changes, the
         * edges will be deleted and replaced with new
         * ones.  Thus this object should be considered temporary only.
         *
         * @param index the index of the desired edge, ranging from 0
         * to getNumberOfEdges()-1 inclusive.
         * @return the requested edge.
         */
        Dim2Edge* getEdge(unsigned long index) const;
        /**
         * Returns the requested face of the given dimension in this
         * triangulation.
         *
         * This template function is to assist with writing dimension-agnostic
         * code that can be reused to work in different dimensions.
         *
         * \pre The template argument \a subdim is between 0 and 2 inclusive.
         *
         * \ifacespython Not present.
         *
         * @param index the index of the desired face, ranging from 0 to
         * getNumberOfFaces<subdim>()-1 inclusive.
         * @return the requested face.
         */
        template <int subdim>
        typename FaceTraits<2, subdim>::Face* getFace(unsigned long index)
            const;
        /**
         * Returns the index of the given boundary component
         * in the triangulation.
         *
         * \pre The given boundary component belongs to this triangulation.
         *
         * \warning Passing a null pointer to this routine will probably
         * crash your program.
         *
         * @param bc specifies which boundary component to find in the
         * triangulation.
         * @return the index of the specified boundary component,
         * where 0 is the first boundary component, 1 is the second and so on. 
         */
        long boundaryComponentIndex(const Dim2BoundaryComponent* bc) const;
        /**
         * Returns the index of the given vertex in the triangulation.
         *
         * \pre The given vertex belongs to this triangulation.
         *
         * \warning Passing a null pointer to this routine will probably
         * crash your program.
         *
         * @param vertex specifies which vertex to find in the triangulation.
         * @return the index of the specified vertex, where 0 is the first
         * vertex, 1 is the second and so on.
         */
        long vertexIndex(const Dim2Vertex* vertex) const;
        /**
         * Returns the index of the given edge in the triangulation.
         *
         * \pre The given edge belongs to this triangulation.
         *
         * \warning Passing a null pointer to this routine will probably
         * crash your program.
         *
         * @param edge specifies which edge to find in the triangulation.
         * @return the index of the specified edge, where 0 is the first
         * edge, 1 is the second and so on.
         */
        long edgeIndex(const Dim2Edge* edge) const;
        /**
         * Returns the index of the given face of the given dimension in this
         * triangulation.
         *
         * This template function is to assist with writing dimension-agnostic
         * code that can be reused to work in different dimensions.
         *
         * \pre The template argument \a subdim is between 0 and 2 inclusive.
         * \pre The given face belongs to this triangulation.
         *
         * \warning Passing a null pointer to this routine will probably
         * crash your program.
         *
         * \ifacespython Not present.
         *
         * @param face specifies which face to find in the triangulation.
         * @return the index of the specified face, where 0 is the first
         * \a subdim-face, 1 is the second \a subdim-face, and so on.
         */
        template <int subdim>
        long faceIndex(const typename FaceTraits<2, subdim>::Face* face) const;

        /*@}*/
        /**
         * \name Isomorphism Testing
         */
        /*@{*/

        using NGenericTriangulation<2>::isIsomorphicTo;
        using NGenericTriangulation<2>::isContainedIn;
        using NGenericTriangulation<2>::findAllIsomorphisms;
        using NGenericTriangulation<2>::findAllSubcomplexesIn;
        using NGenericTriangulation<2>::makeCanonical;

        /*@}*/
        /**
         * \name Basic Properties
         */
        /*@{*/

        /**
         * Always returns \c true.
         *
         * This routine determines if this triangulation is valid; however,
         * there is nothing that can go wrong with vertex links in 2-manifold
         * triangulations, and so this routine always returns \c true.
         *
         * This no-op routine is provided for consistency with higher
         * dimensional triangulations, and to assist with writing
         * dimension-agnostic code.
         *
         * @return \c true.
         */
        bool isValid() const;
        /**
         * Returns the Euler characteristic of this triangulation.
         * This will be evaluated as \a V-E+F.
         *
         * @return the Euler characteristic of this triangulation.
         */
        long getEulerChar() const;
        /**
         * Determines if this triangulation is closed.
         * This is the case if and only if it has no boundary components.
         *
         * @return \c true if and only if this triangulation is closed.
         */
        bool isClosed() const;
        /**
         * A dimension-specific alias for hasBoundaryFacets().
         *
         * See hasBoundaryFacets() for further information.
         */
        bool hasBoundaryEdges() const;
        /**
         * Returns the number of boundary edges in this triangulation.
         *
         * @return the total number of boundary edges.
         */
        unsigned long getNumberOfBoundaryEdges() const;
        /**
         * Always returns \c false.
         *
         * This routine determines if this triangulation is ideal (has a
         * non-trivial vertex link); however, every vertex link in a
         * 2-manifold triangulation is either the interval or the
         * circle, and so ideal triangulations cannot exist.
         * Therefore this routine always returns \c false.
         *
         * This no-op routine is provided for consistency with higher
         * dimensional triangulations, and to assist with writing
         * dimension-agnostic code.
         *
         * @return \c false.
         */
        bool isIdeal() const;

        /**
         * Determines whether this is a minimal triangulation of the
         * underlying 2-manifold; that is, it uses the fewest possible
         * triangles.
         *
         * Testing for minimality is simple in two dimensions (unlike
         * higher dimensions, where it becomes extremely difficult).
         * With the exception of the sphere, disc and projective plane
         * (which require a minimum of 2, 1 and 2 triangles respectively),
         * a closed triangulation is minimal if and only if it has one
         * vertex, and a bounded triangulation is minimal if and only if
         * it has one vertex per boundary component and no internal vertices.
         *
         * The proof is based on a simple Euler characteristic calculation,
         * whereby the number of triangles <tt>T</tt> is
         * <tt>T = 2Vi + Vb - 2C</tt>, where <tt>Vi</tt> and <tt>Vb</tt>
         * are the number of internal and boundary vertices respectively,
         * and where <tt>C</tt> is the Euler characteristic of the
         * underlying manifold.
         *
         * @return \c true if and only if this is a minimal triangulation.
         */
        bool isMinimal() const;

        /*@}*/
        /**
         * \name Skeletal Transformations
         */
        /*@{*/

        /**
         * Checks the eligibility of and/or performs a 1-3 move
         * upon the given triangle.
         * This involves replacing one triangle with three triangles:
         * each new triangle runs from one edge of
         * the original triangle to a new common internal degree three vertex.
         *
         * This move can always be performed.  The \a check argument is
         * present (as for other moves), but is simply ignored (since
         * the move is always legal).  The \a perform argument is also
         * present for consistency with other moves, but if it is set to
         * \c false then this routine does nothing and returns no useful
         * information.
         *
         * Note that after performing this move, all skeletal objects
         * (edges, components, etc.) will be reconstructed, which means
         * any pointers to old skeletal objects (such as the argument \a t)
         * can no longer be used.
         *
         * \pre The given triangle is a triangle of this triangulation.
         *
         * @param t the triangle about which to perform the move.
         * @param check this argument is ignored, since this move is
         * always legal (see the notes above).
         * @param perform \c true if we are to perform the move
         * (defaults to \c true).
         * @return \c true always.
         */
        bool oneThreeMove(Dim2Triangle* t, bool check = true,
            bool perform = true);

        /*@}*/
        /**
         * \name Exporting Triangulations
         */
        /*@{*/

        using NGenericTriangulation<2>::isoSig;

        /*@}*/
        /**
         * \name Importing Triangulations
         */
        /*@{*/

        using NGenericTriangulation<2>::fromIsoSig;
        using NGenericTriangulation<2>::isoSigComponentSize;

        /*@}*/

        static NXMLPacketReader* getXMLReader(NPacket* parent,
            NXMLTreeResolver& resolver);

    protected:
        virtual NPacket* internalClonePacket(NPacket* parent) const;
        virtual void writeXMLPacketData(std::ostream& out) const;

        /**
         * Turns this triangulation into a clone of the given triangulation.
         * The tree structure and label of this triangulation are not touched.
         *
         * @param from the triangulation from which this triangulation
         * will be cloned.
         */
        void cloneFrom(const Triangulation& from);

    private:
        /**
         * Clears any calculated properties and declares them all
         * unknown.  All dynamic memory used for storing known
         * properties is deallocated.
         *
         * In most cases this routine is followed immediately by firing
         * a packet change event.
         */
        virtual void clearAllProperties();

        void deleteSkeleton();
        void calculateSkeleton() const;

        /**
         * Internal to calculateSkeleton().  See the comments within
         * calculateSkeleton() for precisely what this routine does.
         */
        void calculateEdges() const;
        /**
         * Internal to calculateSkeleton().  See the comments within
         * calculateSkeleton() for precisely what this routine does.
         */
        void calculateVertices() const;
        /**
         * Internal to calculateSkeleton().  See the comments within
         * calculateSkeleton() for precisely what this routine does.
         */
        void calculateBoundary() const;

        /**
         * Determines if an isomorphic copy of this triangulation is
         * contained within the given triangulation.
         *
         * If the argument \a completeIsomorphism is \c true, the
         * isomorphism must be onto and boundary complete.
         * That is, this triangulation must be combinatorially
         * isomorphic to the given triangulation.
         *
         * If the argument \a completeIsomorphism is \c false, the
         * isomorphism may be boundary incomplete and may or may not be
         * onto.  That is, this triangulation must appear as a
         * subcomplex of the given triangulation, possibly with some
         * original boundary edges joined to new triangles.
         *
         * See the Dim2Isomorphism class notes for further details
         * regarding boundary complete and boundary incomplete
         * isomorphisms.
         *
         * The isomorphisms found, if any, will be appended to the
         * list \a results.  This list will not be emptied before
         * calculations begin.  All isomorphisms will be newly created,
         * and the caller of this routine is responsible for destroying
         * them.
         *
         * If \a firstOnly is passed as \c true, only the first
         * isomorphism found (if any) will be returned, after which the
         * routine will return immediately.  Otherwise all isomorphisms
         * will be returned.
         *
         * @param other the triangulation in which to search for an
         * isomorphic copy of this triangulation.
         * @param results the list in which any isomorphisms found will
         * be stored.
         * @param completeIsomorphism \c true if isomorphisms must be
         * onto and boundary complete, or \c false if neither of these
         * restrictions should be imposed.
         * @param firstOnly \c true if only one isomorphism should be
         * returned (if any), or \c false if all isomorphisms should be
         * returned.
         * @return the total number of isomorphisms found.
         */
        unsigned long findIsomorphisms(const Triangulation& other,
                std::list<Dim2Isomorphism*>& results,
                bool completeIsomorphism, bool firstOnly) const;

        /**
         * Internal to findIsomorphisms().
         *
         * Examines properties of the given triangle to find any
         * immediate evidence that \a src may not map to \a dest in a
         * boundary complete isomorphism (in which the vertices of \a src
         * are mapped to the vertices of \a dest according to the
         * permutation \a p).
         *
         * In particular, the degrees of vertices are examined.
         *
         * @param src the first of the two triangles to examine.
         * @param dest the second of the two triangles to examine.
         * @param p the permutation under which the vertices of \a src
         * must map to the vertices of \a dest.
         * @return \c true if no immediate incompatibilities between the
         * triangles were found, or \c false if properties of the
         * triangles were found that differ between \a src and \a dest.
         */
        static bool compatibleTriangles(Dim2Triangle* src, Dim2Triangle* dest,
            NPerm3 p);

    friend class regina::Simplex<2>;
    friend class regina::SimplexBase<2>;
    friend class regina::TriangulationBase<2>;
    friend class regina::NGenericTriangulation<2>;
    friend class regina::NXMLDim2TriangulationReader;
};

/**
 * A convenience typedef for Triangulation<2>.
 */
typedef Triangulation<2> Dim2Triangulation;

/*@}*/

} // namespace regina
// Some more headers that are required for inline functions:
#include "dim2/dim2triangle.h"
#include "dim2/dim2edge.h"
#include "dim2/dim2vertex.h"
#include "dim2/dim2boundarycomponent.h"
namespace regina {

// Inline functions for Triangulation<2>

inline Triangulation<2>::Triangulation() {
}

inline Triangulation<2>::Triangulation(const Triangulation& cloneMe) {
    cloneFrom(cloneMe);
}

inline Triangulation<2>::~Triangulation() {
    clearAllProperties();
}

inline void Triangulation<2>::writeTextShort(std::ostream& out) const {
    out << "Triangulation with " << simplices_.size()
        << (simplices_.size() == 1 ? " triangle" : " triangles");
}

inline bool Triangulation<2>::dependsOnParent() const {
    return false;
}

inline unsigned long Triangulation<2>::getNumberOfTriangles() const {
    return simplices_.size();
}

inline const std::vector<Dim2Triangle*>& Triangulation<2>::getTriangles()
        const {
    return (const std::vector<Dim2Triangle*>&)(simplices_);
}

inline Dim2Triangle* Triangulation<2>::getTriangle(unsigned long index) {
    return simplices_[index];
}

inline const Dim2Triangle* Triangulation<2>::getTriangle(unsigned long index)
        const {
    return simplices_[index];
}

inline long Triangulation<2>::triangleIndex(const Dim2Triangle* tri) const {
    return tri->markedIndex();
}

inline Dim2Triangle* Triangulation<2>::newTriangle() {
    return newSimplex();
}

inline Dim2Triangle* Triangulation<2>::newTriangle(const std::string& desc) {
    return newSimplex(desc);
}

inline void Triangulation<2>::removeTriangle(Dim2Triangle* tri) {
    removeSimplex(tri);
}

inline void Triangulation<2>::removeTriangleAt(unsigned long index) {
    removeSimplexAt(index);
}

inline void Triangulation<2>::removeAllTriangles() {
    removeAllSimplices();
}

inline unsigned long Triangulation<2>::getNumberOfBoundaryComponents() const {
    ensureSkeleton();
    return boundaryComponents_.size();
}

inline unsigned long Triangulation<2>::getNumberOfVertices() const {
    ensureSkeleton();
    return vertices_.size();
}

inline unsigned long Triangulation<2>::getNumberOfEdges() const {
    ensureSkeleton();
    return edges_.size();
}

template <>
inline unsigned long Triangulation<2>::getNumberOfFaces<0>() const {
    return getNumberOfVertices();
}

template <>
inline unsigned long Triangulation<2>::getNumberOfFaces<1>() const {
    return getNumberOfEdges();
}

template <>
inline unsigned long Triangulation<2>::getNumberOfFaces<2>() const {
    return getNumberOfTriangles();
}

inline const std::vector<Dim2BoundaryComponent*>&
        Triangulation<2>::getBoundaryComponents() const {
    ensureSkeleton();
    return (const std::vector<Dim2BoundaryComponent*>&)(boundaryComponents_);
}

inline const std::vector<Dim2Vertex*>& Triangulation<2>::getVertices() const {
    ensureSkeleton();
    return (const std::vector<Dim2Vertex*>&)(vertices_);
}

inline const std::vector<Dim2Edge*>& Triangulation<2>::getEdges() const {
    ensureSkeleton();
    return (const std::vector<Dim2Edge*>&)(edges_);
}

inline Dim2BoundaryComponent* Triangulation<2>::getBoundaryComponent(
        unsigned long index) const {
    ensureSkeleton();
    return boundaryComponents_[index];
}

inline Dim2Vertex* Triangulation<2>::getVertex(unsigned long index) const {
    ensureSkeleton();
    return vertices_[index];
}

inline Dim2Edge* Triangulation<2>::getEdge(unsigned long index) const {
    ensureSkeleton();
    return edges_[index];
}

template <>
inline Dim2Vertex* Triangulation<2>::getFace<0>(unsigned long index) const {
    return vertices_[index];
}

template <>
inline Dim2Edge* Triangulation<2>::getFace<1>(unsigned long index) const {
    return edges_[index];
}

template <>
inline Dim2Triangle* Triangulation<2>::getFace<2>(unsigned long index) const {
    return simplices_[index];
}

inline long Triangulation<2>::boundaryComponentIndex(
        const Dim2BoundaryComponent* boundaryComponent) const {
    return boundaryComponent->markedIndex();
}

inline long Triangulation<2>::vertexIndex(const Dim2Vertex* vertex) const {
    return vertex->markedIndex();
}

inline long Triangulation<2>::edgeIndex(const Dim2Edge* edge) const {
    return edge->markedIndex();
}

template <>
inline long Triangulation<2>::faceIndex<0>(const Dim2Vertex* face) const {
    return face->markedIndex();
}

template <>
inline long Triangulation<2>::faceIndex<1>(const Dim2Edge* face) const {
    return face->markedIndex();
}

template <>
inline long Triangulation<2>::faceIndex<2>(const Dim2Triangle* face) const {
    return face->markedIndex();
}

inline bool Triangulation<2>::isValid() const {
    return true;
}

inline long Triangulation<2>::getEulerChar() const {
    ensureSkeleton();

    // Cast away the unsignedness of std::vector::size().
    return static_cast<long>(vertices_.size())
        - static_cast<long>(edges_.size())
        + static_cast<long>(simplices_.size());
}

inline bool Triangulation<2>::isClosed() const {
    ensureSkeleton();
    return boundaryComponents_.empty();
}

inline bool Triangulation<2>::hasBoundaryEdges() const {
    return ! isClosed();
}

inline unsigned long Triangulation<2>::getNumberOfBoundaryEdges() const {
    ensureSkeleton();
    return 2 * edges_.size() - 3 * simplices_.size();
}

inline bool Triangulation<2>::isIdeal() const {
    return false;
}

inline NPacket* Triangulation<2>::internalClonePacket(NPacket*) const {
    return new Triangulation(*this);
}

} // namespace regina

#endif

