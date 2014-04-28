
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Computational Engine                                                  *
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

#include <vector>
#include <map>
#include <sstream>

#include "dim4/dim4triangulation.h"

namespace regina {

void Dim4Triangulation::barycentricSubdivision() {
    unsigned long nOldPent = pentachora_.size();
    if (nOldPent == 0)
        return;

    Dim4Triangulation staging;
    ChangeEventSpan span1(&staging);

    Dim4Pentachoron** newPent = new Dim4Pentachoron*[nOldPent * 120];
    Dim4Pentachoron* oldPent;

    // A pentachoron in the subdivision is uniquely defined by the
    // permutation (tet, triangle, edge, vtx, corner) of (0, 1, 2, 3, 4).
    // This is the pentachoron that:
    // - meets the boundary in the tetrahedron opposite vertex "tet";
    // - meets that tetrahedron in the triangle opposite vertex "triangle";
    // - meets that triangle in the edge opposite vertex "edge";
    // - meets that edge in the vertex opposite vertex "vtx";
    // - directly touches vertex "corner".

    unsigned long pent;
    for (pent=0; pent < 120 * nOldPent; ++pent)
        newPent[pent] = staging.newPentachoron();

    // Do all of the internal gluings
    int permIdx;
    NPerm5 perm, glue;
    for (pent=0; pent < nOldPent; ++pent)
        for (permIdx = 0; permIdx < 120; ++permIdx) {
            perm = NPerm5::S5[permIdx];
            // (0, 1, 2, 3, 4) -> (tet, triangle, edge, vtx, corner)

            // Internal gluings within the old pentachoron:
            newPent[120 * pent + permIdx]->joinTo(perm[4],
                newPent[120 * pent + (perm * NPerm5(4, 3)).S5Index()],
                NPerm5(perm[4], perm[3]));

            newPent[120 * pent + permIdx]->joinTo(perm[3],
                newPent[120 * pent + (perm * NPerm5(3, 2)).S5Index()],
                NPerm5(perm[3], perm[2]));

            newPent[120 * pent + permIdx]->joinTo(perm[2],
                newPent[120 * pent + (perm * NPerm5(2, 1)).S5Index()],
                NPerm5(perm[2], perm[1]));

            newPent[120 * pent + permIdx]->joinTo(perm[1],
                newPent[120 * pent + (perm * NPerm5(1, 0)).S5Index()],
                NPerm5(perm[1], perm[0]));

            // Adjacent gluings to the adjacent pentachoron:
            oldPent = getPentachoron(pent);
            if (! oldPent->adjacentPentachoron(perm[0]))
                continue; // This hits a boundary facet.
            if (newPent[120 * pent + permIdx]->adjacentPentachoron(perm[0]))
                continue; // We've already done this gluing from the other side.

            glue = oldPent->adjacentGluing(perm[0]);
            newPent[120 * pent + permIdx]->joinTo(perm[0],
                newPent[120 * pentachoronIndex(
                    oldPent->adjacentPentachoron(perm[0])) +
                    (glue * perm).S5Index()],
                glue);
        }


    // Delete the existing pentachora and put in the new ones.
    ChangeEventSpan span2(this);
    removeAllPentachora();
    swapContents(staging);
    delete[] newPent;
}

 // NOTE: the class subDivNot below should be local to Dim4Triangulation::idealToFinite but
 // this is not allowed until we move to C++0x as templates can't be instantantiated
 // with local types in the current version of GCC.  TODO: Might have to put something in 
 // here to ensure subDivNot does not make it into the Regina library.
 //
 // We list the types of pentachora:
 // (1) Original pentachoron, i.e. no ideal vertices. 
 // else pen has an ideal vertex. Pen coords.
 // (2) Cone on standard tetrahedron, at pent barycentre. Using ambient pent coords.
 // (3) Cone on ideal tetrahedron, at pent barycentre. Using ambient pent coords.
 // Remaining are cone at pent barycentre of subdivided tetrahedron, ON:
 // (4) Cone on std tri at tet barycentre, using tet coords 
 // (5) Cone on ideal tri at tet barycentre, using tet coords
 // else 
 // (6) cone at tet barycentre of subdivided triangle with ideal vertices, using tri coords.
 enum subDivType { _OP, _CT, _CiT, _CCt, _CCit, _CCdt };
 //                 (1) (2)   (3)   (4)   (5)     (6)

 struct subDivNot { // used for types (a) and (b) via tetFlag.
  subDivType penType;
  unsigned long penIdx; 
  unsigned long tetIdx; // needed for all but (1).
  unsigned long triIdx; // needed for (4), (5), (6).
  unsigned long vtxIdx; // needed to specify which triangle vertex for (6).

 subDivNot( const subDivNot &cloneMe ) {
    penType = cloneMe.penType;
    penIdx = cloneMe.penIdx; tetIdx = cloneMe.tetIdx; 
    triIdx = cloneMe.triIdx; vtxIdx = cloneMe.vtxIdx;
   }

 // final three options optional. 
 subDivNot( subDivType PT, unsigned long PI, unsigned long TI=0, 
                           unsigned long tI=0, unsigned long VI=0 )
  { penType = PT; penIdx = PI; tetIdx = TI; triIdx = tI; vtxIdx = VI; }
 
 bool operator<(const subDivNot &other) const {
  if (penIdx < other.penIdx) return true;  
  if (penIdx > other.penIdx) return false;
  if (penType < other.penType) return true; 
  if (penType > other.penType) return false;
  // same penType here.
  if (penType == _OP) return false;
  if (tetIdx < other.tetIdx) return true;  
  if (tetIdx > other.tetIdx) return false;
  if ( (penType == _CT) || (penType == _CiT) ) return false; 
  // done with (1), (2), (3)
  if (triIdx < other.triIdx) return true;  
  if (triIdx > other.triIdx) return false;
  if ( (penType==_CCt) || (penType==_CCit) ) return false; 
  // only _CCdt (6) left
  if (vtxIdx < other.vtxIdx) return true; 
  if (vtxIdx > other.vtxIdx) return false;
  return false;  }

 std::string label() const
  {
   std::stringstream retval;
   if (penType==_OP)   retval<<"OP."   <<penIdx; else //1
   if (penType==_CT)   retval<<"CT.P"  <<penIdx<<"T"<<tetIdx; else //2
   if (penType==_CiT)  retval<<"CiT.P" <<penIdx<<"v"<<tetIdx; else //3
   if (penType==_CCt)  retval<<"CCt.P" <<
    penIdx<<"T"<<tetIdx<<"t"<<triIdx; else //4
   if (penType==_CCit) retval<<"CCit.P"<<
    penIdx<<"T"<<tetIdx<<"v"<<triIdx; else //5
   if (penType==_CCdt) retval<<"CCdt.P"<<
    penIdx<<"T"<<tetIdx<<"t"<<triIdx<<"v"<<vtxIdx; //6
   return retval.str();
  }
 }; // end def subDivNot

bool Dim4Triangulation::idealToFinite()
{
 bool idVrts(false);
 for (unsigned long i=0; i<getNumberOfVertices(); i++)
  if (getVertex(i)->isIdeal()) { idVrts=true; break; }
 if (!idVrts) return false;
 // * * * Create new triangulation * * *
 Dim4Triangulation* newTri( new Dim4Triangulation );
 #ifdef DEBUG
 std::cerr << "Performing idealToFinite()\n";
 #endif

 // * * * Create the pentachora for the new triangulation * * * 
 std::map< subDivNot, Dim4Pentachoron* > newPens; // this will index them. 
 for (unsigned long i=0; i<getNumberOfPentachora(); i++)
  {
   const Dim4Pentachoron* aPen( getPentachoron(i) ); // ambient pent
   bool pIv(false);   // check if has ideal vertices
   for (unsigned long j=0; j<5; j++) if (aPen->getVertex(j)->isIdeal()) 
    pIv = true;
   if (!pIv) { newPens.insert( std::pair< subDivNot, Dim4Pentachoron* >
     ( subDivNot( _OP, i ), newTri->newPentachoron() ) ); continue; }
   for (unsigned long j=0; j<5; j++) // tet / pen vtx loop
    {
     // _CiT check
     if (aPen->getVertex(j)->isIdeal()) 
        newPens.insert( std::pair< subDivNot, Dim4Pentachoron* >
        ( subDivNot( _CiT, i, j ), newTri->newPentachoron() ) );
     // _CT check 
     bool TIv(false); // tet across from j has ideal vertex? 
     for (unsigned long k=1; k<5; k++) 
        if (aPen->getVertex( (j+k) % 5)->isIdeal())
        TIv = true;
     if (!TIv) { newPens.insert( std::pair< subDivNot, Dim4Pentachoron* >
       ( subDivNot( _CT, i, j ), newTri->newPentachoron() ) ); continue; }
     // we're in situation 4, 5, or 6.
     const Dim4Tetrahedron* aTet( aPen->getTetrahedron(j) );
     for (unsigned long k=0; k<4; k++)
      {
       if (aTet->getVertex(k)->isIdeal()) 
        newPens.insert( std::pair< subDivNot, Dim4Pentachoron* >
        ( subDivNot( _CCit, i, j, k ), newTri->newPentachoron() ) ); // CCit
       newPens.insert( std::pair< subDivNot, Dim4Pentachoron* > // CCt
        ( subDivNot( _CCt, i, j, k ), newTri->newPentachoron() ) );
       bool tIv(false); // check if remaining triangle has ideal vertex or not.
       for (unsigned long l=1; l<4; l++) 
        if (aTet->getVertex( (k+l) % 4)->isIdeal()) tIv = true;
       if (!tIv) continue;
       // the only way we can get here is the triangle has ideal vertices.  
       // So we have to subdivide canonically.  
       const Dim4Triangle* aTri( aTet->getTriangle(k) );
       for (unsigned long l=0; l<3; l++) if (aTri->getVertex(l)->isIdeal() )
       newPens.insert( std::pair< subDivNot, Dim4Pentachoron* >
        ( subDivNot( _CCdt, i, j, k, l ), newTri->newPentachoron() ) );   
      } // end k loop
    } // end j loop
  } // end i loop

 //                             * * Create the Gluings. * * 
 //      * * * gluings corresponding to non-boundary tets in original tri * * *
 for (unsigned long i=0; i<getNumberOfTetrahedra(); i++) 
   if (!getTetrahedron(i)->isBoundary())
  { // check if has ideal vertices
   const Dim4Tetrahedron* aTet( getTetrahedron(i) );
   const Dim4TetrahedronEmbedding tEmb0( aTet->getEmbedding(0) );
   const Dim4TetrahedronEmbedding tEmb1( aTet->getEmbedding(1) );

   bool TIv(false);
   for (unsigned long j=0; j<4; j++) if (aTet->getVertex(j)->isIdeal()) 
    { TIv=true; break; }
   if (!TIv) // decide between _OP (1) and _CT (2) for these
    {
     subDivNot p0( _OP, pentachoronIndex( tEmb0.getPentachoron() ) );  
     subDivNot p1( _OP, pentachoronIndex( tEmb1.getPentachoron() ) ); 
     if (tEmb0.getPentachoron()->getVertex( tEmb0.getTetrahedron() )->isIdeal())
      { p0.penType = _CT; p0.tetIdx = aTet->getEmbedding(0).getTetrahedron(); }
     if (tEmb1.getPentachoron()->getVertex( tEmb1.getTetrahedron() )->isIdeal())
      { p1.penType = _CT; p1.tetIdx = aTet->getEmbedding(1).getTetrahedron(); }
      #ifdef DEBUG // test to check if p0 and p1 exist 
      if (newPens.find(p0)==newPens.end()) std::cerr<<"idealToFinite (1) p0 DNE";
      if (newPens.find(p1)==newPens.end()) std::cerr<<"idealToFinite (1) p1 DNE";
      if (newPens[p0]->adjacentPentachoron( tEmb0.getTetrahedron() )!=NULL)
         std::cerr<<"idealToFinite (1) p0 GLUED";
      if (newPens[p1]->adjacentPentachoron( tEmb1.getTetrahedron() )!=NULL)
         std::cerr<<"idealToFinite (1) p1 GLUED";
      #endif // and if gluings previously set. 
     newPens[ p0 ]->joinTo( tEmb0.getTetrahedron(), newPens[ p1 ], 
        tEmb0.getPentachoron()->adjacentGluing( tEmb0.getTetrahedron() ) );
     continue;
    }
   // tet has ideal vertices, so it consists of cones on (perhaps subdivided) 
   // triangles, so we're we're gluing (4) tet coords (5) tc also  
   // or (6) tri coords.
   for (unsigned long j=0; j<4; j++) 
    {
     bool tIV(false);   // check if tri across from vertex j has ideal vertices
     for (unsigned long k=1; k<4; k++) 
        if (aTet->getVertex( (j+k) % 4 )->isIdeal()) { tIV=true; break; }
     {
     subDivNot p0( _CCt, pentachoronIndex( tEmb0.getPentachoron() ) ); // might have to 
     subDivNot p1( _CCt, pentachoronIndex( tEmb1.getPentachoron() ) ); // modify later
     p0.tetIdx = tEmb0.getTetrahedron(); p1.tetIdx = tEmb1.getTetrahedron();
     p0.triIdx = j; p1.triIdx = j;
      #ifdef DEBUG // test to check if p0 and p1 exist 
      if (newPens.find(p0)==newPens.end()) std::cerr<<"idealToFinite (2) p0 DNE";
      if (newPens.find(p1)==newPens.end()) std::cerr<<"idealToFinite (2) p1 DNE";
      if (newPens[p0]->adjacentPentachoron( 4 )!=NULL)
         std::cerr<<"idealToFinite (2) p0 GLUED";
      if (newPens[p1]->adjacentPentachoron( 4 )!=NULL)
         std::cerr<<"idealToFinite (2) p1 GLUED";
      #endif // and if gluings previously set. 
     newPens[ p0 ]->joinTo( 4, newPens[ p1 ], NPerm5() );
     if (!tIV) continue;

     const Dim4Triangle* aTri(aTet->getTriangle(j) );
     // now the type (6) _CCdt.
     p0.penType = _CCdt; p1.penType = _CCdt;
     for (unsigned long k=0; k<3; k++) if (aTri->getVertex(k)->isIdeal())
      {
       p0.vtxIdx = k; p1.vtxIdx = k; // the tri with no ideal vertices. 
     #ifdef DEBUG // test to check if p0 and p1 exist 
     if (newPens.find(p0)==newPens.end()) std::cerr<<"idealToFinite (3) p0 DNE";
     if (newPens.find(p1)==newPens.end()) std::cerr<<"idealToFinite (3) p1 DNE";
     if (newPens[p0]->adjacentPentachoron( 4 )!=NULL)
      std::cerr<<"idealToFinite (3) p0 GLUED";
     if (newPens[p1]->adjacentPentachoron( 4 )!=NULL)
      std::cerr<<"idealToFinite (3) p1 GLUED";
     #endif // and if gluings previously set. 
       newPens[ p0 ]->joinTo( 4, newPens[ p1 ], NPerm5() );
      }     
     }
     if (aTet->getVertex(j)->isIdeal()) // we have a _CCit
      { 
       subDivNot p0( _CCit, pentachoronIndex( tEmb0.getPentachoron() ) ); 
       subDivNot p1( _CCit, pentachoronIndex( tEmb1.getPentachoron() ) ); 
       p0.tetIdx = tEmb0.getTetrahedron(); p1.tetIdx = tEmb1.getTetrahedron();
       p0.triIdx = j; p1.triIdx = j;
     #ifdef DEBUG // test to check if p0 and p1 exist 
     if (newPens.find(p0)==newPens.end()) std::cerr<<"idealToFinite (4) p0 DNE";
     if (newPens.find(p1)==newPens.end()) std::cerr<<"idealToFinite (4) p1 DNE";
     if (newPens[p0]->adjacentPentachoron( 4 )!=NULL)
      std::cerr<<"idealToFinite (4) p0 GLUED";
     if (newPens[p1]->adjacentPentachoron( 4 )!=NULL)
      std::cerr<<"idealToFinite (4) p1 GLUED";
     #endif // and if gluings previously set. 
       newPens[ p0 ]->joinTo( 4, newPens[ p1 ], NPerm5() );
      }
    } // end loop through tet vertices
  } // end look through tets.

 // * * * gluings corresponding to subdivision of individual pentachora * * *
 for (unsigned long i=0; i<getNumberOfPentachora(); i++)
  {
   const Dim4Pentachoron* aPen( getPentachoron(i) );
   bool pIv(false);
   for (unsigned long j=0; j<5; j++) 
        if (aPen->getVertex(j)->isIdeal()) pIv = true;
   if (!pIv) continue; // nothing to do!
   // step 1: all the gluings corresponding to triangle subdivisions, i.e. 
   //    all  objects of type (6) CCdt and (4) CCt if on a common pen, tet and triangle. 
   for (unsigned long j=0; j<5; j++)
    {
     const Dim4Tetrahedron* aTet( aPen->getTetrahedron( j ) );
     for (unsigned long k=0; k<4; k++)
      {
       subDivNot p0( _OP, i, j, k ); // will have to 
       subDivNot p1( _OP, i, j, k ); // modify later
       const Dim4Triangle* aTri( aTet->getTriangle(k) );
       bool tidV(false);
       for (unsigned long l=0; l<3; l++) if (aTri->getVertex(l)->isIdeal()) 
        { tidV = true; break; }
       if (!tidV) continue;
       // the triangle has ideal vertices, so there's something to do.
       // gluing pattern CCdt 0 -- CCdt 2 -- CCt -- CCdt 1, if not ideal just erase the
       // CCt uses tet coords.  So we need the tri inclusion.
       NPerm5 triInc( aTet->getTriangleMapping(k) );
       if (aTri->getVertex(1)->isIdeal()) // glue this CCdt to CCt.
        { p0.penType = _CCdt; p1.penType = _CCt; 
          p0.vtxIdx = 1; 
     #ifdef DEBUG // test to check if p0 and p1 exist 
     if (newPens.find(p0)==newPens.end()) std::cerr<<"idealToFinite (4) p0 DNE";
     if (newPens.find(p1)==newPens.end()) std::cerr<<"idealToFinite (4) p1 DNE";
     if (newPens[p0]->adjacentPentachoron( 1 )!=NULL)
      std::cerr<<"idealToFinite (4) p0 GLUED";
     if (newPens[p1]->adjacentPentachoron( triInc[2] )!=NULL)
      std::cerr<<"idealToFinite (4) p1 GLUED";
     #endif // and if gluings previously set. 
          newPens[ p0 ]->joinTo( 1, newPens[ p1 ], 
             NPerm5(triInc[0], triInc[2], triInc[1], triInc[3], triInc[4] ) );
        }
       if (aTri->getVertex(2)->isIdeal()) // glue this CCdt to CCt.
        { p0.penType = _CCdt; p1.penType = _CCt;
          p0.vtxIdx  = 2; 
     #ifdef DEBUG // test to check if p0 and p1 exist 
     if (newPens.find(p0)==newPens.end()) std::cerr<<"idealToFinite (5) p0 DNE";
     if (newPens.find(p1)==newPens.end()) std::cerr<<"idealToFinite (5) p1 DNE";
     if (newPens[p0]->adjacentPentachoron( 2 )!=NULL)
      std::cerr<<"idealToFinite (5) p0 GLUED";
     if (newPens[p1]->adjacentPentachoron( triInc[1] )!=NULL)
      std::cerr<<"idealToFinite (5) p1 GLUED";
     #endif // and if gluings previously set. 
          newPens[ p0 ]->joinTo( 2, newPens[ p1 ], 
            NPerm5( triInc[0], triInc[2], triInc[1], triInc[3], triInc[4] ) );
        }
       if (aTri->getVertex(0)->isIdeal() && aTri->getVertex(2)->isIdeal()) // glue 0 to 2
        { p0.penType = _CCdt; p1.penType = _CCdt; 
          p0.vtxIdx = 2;      p1.vtxIdx = 0; 
    #ifdef DEBUG // test to check if p0 and p1 exist 
    if (newPens.find(p0)==newPens.end()) std::cerr<<"idealToFinite (6) p0 DNE";
    if (newPens.find(p1)==newPens.end()) std::cerr<<"idealToFinite (6) p1 DNE";
    if (newPens[p0]->adjacentPentachoron( 1 )!=NULL)
      std::cerr<<"idealToFinite (6) p0 GLUED";
    if (newPens[p1]->adjacentPentachoron( 2 )!=NULL)
      std::cerr<<"idealToFinite (6) p1 GLUED";
    #endif // and if gluings previously set. 
          newPens[ p0 ]->joinTo( 1, newPens[ p1 ], NPerm5(0, 2, 1, 3, 4) );
        } 
       if (aTri->getVertex(0)->isIdeal() && !aTri->getVertex(2)->isIdeal()) // glue 0 CCdt to CCt
        { p0.penType = _CCdt; p1.penType = _CCt; 
          p0.vtxIdx = 0;      p1.vtxIdx = 0;
     #ifdef DEBUG // test to check if p0 and p1 exist 
     if (newPens.find(p0)==newPens.end()) std::cerr<<"idealToFinite (7) p0 DNE";
     if (newPens.find(p1)==newPens.end()) std::cerr<<"idealToFinite (7) p1 DNE";
     if (newPens[p0]->adjacentPentachoron( 2 )!=NULL)
      std::cerr<<"idealToFinite (7) p0 GLUED";
     if (newPens[p1]->adjacentPentachoron( triInc[1] )!=NULL)
      std::cerr<<"idealToFinite (7) p1 GLUED";
     #endif // and if gluings previously set. 
          newPens[ p0 ]->joinTo( 2, newPens[ p1 ], 
            NPerm5( triInc[0], triInc[2], triInc[1], triInc[3], triInc[4] ) );
        }
      }
    }

   // step 2: glue the types (6) CCdt, (4) CCt and (5) CCit if on common tet
   //  but not on common triangular face of a tet. One gluing for every edge
   //  of the tet, and for every ideal edge of a triangle in the tet, if it exists.
   for (unsigned long j=0; j<5; j++)
    {
     const Dim4Tetrahedron* aTet( aPen->getTetrahedron(j) );
     bool tIv(false);
     for (unsigned long k=0; k<4; k++) 
        if (aTet->getVertex(k)->isIdeal()) { tIv=true; break; }
     if (!tIv) continue;
     // first, we run through the tets of this pentachoron, and check if it has
     // ideal vertices.  If not, jump to next step. 
     // (a) glue the CCit's (tet coords) to the CCdt's and CCts (tri and tet coords). 
     for (unsigned long k=0; k<4; k++) if (aTet->getVertex(k)->isIdeal())
      { 
       subDivNot p0( _CCdt, i, j ); // will have to 
       subDivNot p1( _CCit, i, j, k ); // modify later
       for (unsigned long l=1; l<4; l++)
         {
          p0.triIdx = (k+l) % 4; 
          const Dim4Triangle* aTri( aTet->getTriangle( (k+l) % 4 ) );
          NPerm5 triInc( aTet->getTriangleMapping( (k+l) % 4 ) );
          p0.vtxIdx = triInc.preImageOf( k );
          // figure out gluing map, would seem to depend on p1.vtxIdx non-trivially.
          // p0.vtxIdx= 0, 20    0->triInc[1] 1->p0.triIdx 2->triInc[2] 3->k 4->4
          //            1, 12    0->p0.triIdx 1->triInc[0] 2->triInc[2] 3->k 4->4
          //            2, 12    0->p0.triIdx 1->triInc[1] 2->triInc[0] 3->k 4->4
          unsigned long A( (p0.vtxIdx==0) ? triInc[1] : p0.triIdx );
          unsigned long B( (p0.vtxIdx==0) ? p0.triIdx : (p0.vtxIdx==1) ? 
            triInc[0] : triInc[1] );
          unsigned long C( (p0.vtxIdx==2) ? triInc[0] : triInc[2] );
     #ifdef DEBUG // test to check if p0 and p1 exist 
     if (newPens.find(p0)==newPens.end()) std::cerr<<"idealToFinite (8) p0 DNE";
     if (newPens.find(p1)==newPens.end()) std::cerr<<"idealToFinite (8) p1 DNE";
     if (newPens[p0]->adjacentPentachoron( (p0.vtxIdx==0) ? 1 : 0 )!=NULL)
      std::cerr<<"idealToFinite (8) p0 GLUED";
     if (newPens[p1]->adjacentPentachoron( (p0.vtxIdx==0) ? B : A )!=NULL)
      std::cerr<<"idealToFinite (8) p1 GLUED";
     #endif // and if gluings previously set. 
          newPens[ p0 ]->joinTo( (p0.vtxIdx==0) ? 1 : 0, newPens[ p1 ], 
            NPerm5( A, B, C, k, 4 ) );
         }        
      }
     // (b) glue the CCdt and CCt's appropriately across tetrahedral edges.
     for (unsigned long k=0; k<6; k++)
      { // recall aTet is the ambient tet, as we're in the j loop.
        subDivNot p0( _OP, i, j ); // will have to 
        subDivNot p1( _OP, i, j ); // modify later
        NPerm5 eMap( aTet->getEdgeMapping(k) ); // eMap[0] eMap[1] edge endpts,
         // eMap[2], eMap[3] tri indices.
        NPerm5 triInc2( aTet->getTriangleMapping( eMap[2] ) ); // natural inclusion to tet.
        NPerm5 triInc3( aTet->getTriangleMapping( eMap[3] ) ); // also. 
        p0.triIdx = eMap[2]; p1.triIdx = eMap[3];
        NPerm5 incPerm0, incPerm1; // relating the pent facets to the ambient pent.
        unsigned long glueT(0); // glue across this tet in pen p0.
        // for tri2 we are gluing a CCt only if either:
        //  (1) triInc2.preImageOf( eMap[3] )==0 or 
        //  (2) triInc2[1] non-ideal
        //  (3) *both* triInc2[2] and triInc2[0] nonideal
        if (eMap[3]==triInc2[0] ) // id vtx 0 in tri2 adj  
            { glueT=triInc2[0]; p0.penType = _CCt;  } else 
        if (!aTet->getVertex( triInc2[1] )->isIdeal() &&
            eMap[3]==triInc2[2] ) // id vtx 1 in tri2 adj
            { glueT=triInc2[2]; p0.penType = _CCt;  } else // id vtx
        if (!aTet->getVertex( triInc2[0] )->isIdeal() && 
            !aTet->getVertex( triInc2[2] )->isIdeal() &&
            eMap[3]==triInc2[1] )
            { glueT=triInc2[1]; p0.penType = _CCt;  } else // now the _CCdt
        if (aTet->getVertex( triInc2[1] )->isIdeal() &&
            eMap[3]==triInc2[2] ) // CCdt vtx 1 
           { glueT=2; p0.penType = _CCdt; incPerm0 = triInc2; p0.vtxIdx = 1; } else
        if ( aTet->getVertex( triInc2[2] )->isIdeal() &&
            !aTet->getVertex( triInc2[0] )->isIdeal() &&
             eMap[3]==triInc2[1] ) 
           { glueT=1; p0.penType = _CCdt; incPerm0 = triInc2; p0.vtxIdx = 2; } else
        if ( aTet->getVertex( triInc2[0] )->isIdeal() &&
             eMap[3]==triInc2[1] ) 
           { glueT=0; p0.penType = _CCdt; 
             incPerm0 = NPerm5(triInc2[1],triInc2[2],triInc2[0], triInc2[3], triInc2[4] ); }
        else { std::cout<<"Dim4Triangulation::idealToFinite() Error 1."<<std::endl; exit(1); } 
        if (eMap[2]==triInc3[0]) // repeating the above for the other side.
            { p1.penType = _CCt; } else
        if (!aTet->getVertex( triInc3[1] )->isIdeal() &&
            eMap[2]==triInc3[2] ) // id vtx 1 in tri2 adj
            { p1.penType = _CCt;   } else // id vtx
        if (!aTet->getVertex( triInc3[0] )->isIdeal() && 
            !aTet->getVertex( triInc3[2] )->isIdeal() &&
            eMap[2]==triInc3[1] )
            { p1.penType = _CCt;  } else // now the _CCdt
        if (aTet->getVertex( triInc3[1] )->isIdeal() &&
            eMap[2]==triInc3[2] ) // CCdt vtx 1 
           { p1.penType = _CCdt; incPerm1 = triInc3; p1.vtxIdx = 1; } else
        if ( aTet->getVertex( triInc3[2] )->isIdeal() &&
            !aTet->getVertex( triInc3[0] )->isIdeal() &&
             eMap[2]==triInc3[1] ) 
           { p1.penType = _CCdt; incPerm1 = triInc3; p1.vtxIdx = 2;  } else
        if ( aTet->getVertex( triInc3[0] )->isIdeal() &&
             eMap[2]==triInc3[1] ) 
           { p1.penType = _CCdt; 
             incPerm1 = NPerm5(triInc3[1],triInc3[2],triInc3[0], triInc3[3], triInc3[4] );  }
        else { std::cout<<"Dim4Triangulation::idealToFinite() Error 2."<<std::endl; exit(1); } 
           #ifdef DEBUG // test to check if p0 and p1 exist 
           if (newPens.find(p0)==newPens.end()) std::cerr<<"idealToFinite (9) p0 DNE";
           if (newPens.find(p1)==newPens.end()) std::cerr<<"idealToFinite (9) p1 DNE";
           if (newPens[p0]->adjacentPentachoron( glueT )!=NULL)
            std::cerr<<"idealToFinite (9) p0 GLUED";
           if (newPens[p1]->adjacentPentachoron( incPerm1.inverse()[NPerm5(eMap[2],eMap[3])[incPerm0[glueT]]] )!=NULL)
            std::cerr<<"idealToFinite (9) p1 GLUED";
           #endif // and if gluings previously set. 
        newPens[ p0 ]->joinTo( glueT, newPens[ p1 ], 
            incPerm1.inverse()*NPerm5(eMap[2], eMap[3])*incPerm0 );
      } 
    }

   // step 3: glue types in common pen but not in common tet facet. 
   // we have an ideal vertex, so this pen is subdivided at the barycentre. 
   for (unsigned long j=0; j<5; j++) if (aPen->getVertex(j)->isIdeal())
    { // **all** CiT type (4) gluings done 
     subDivNot p0( _CiT, i, j ); // uses pen coords
     subDivNot p1( _CCit, i );   // uses tet coords. 
     for (unsigned long k=1; k<5; k++) // gluing for tet j+k % 5.
      {
       NPerm5 tetInc( aPen->getTetrahedronMapping( (j+k) % 5 ) );
       const Dim4Tetrahedron* aTet( aPen->getTetrahedron( (j+k) % 5 ) );
       p1.tetIdx = (j+k) % 5;
       p1.triIdx = tetInc.preImageOf( j ); // the ideal triangle
        #ifdef DEBUG // test to check if p0 and p1 exist 
        if (newPens.find(p0)==newPens.end()) std::cerr<<"idealToFinite (10) p0 DNE";
        if (newPens.find(p1)==newPens.end()) std::cerr<<"idealToFinite (10) p1 DNE";
        if (newPens[p0]->adjacentPentachoron( (j+k)%5 )!=NULL)
         std::cerr<<"idealToFinite (10) p0 GLUED";
        if (newPens[p1]->adjacentPentachoron(tetInc.inverse()[j] )!=NULL)
         std::cerr<<"idealToFinite (10) p1 GLUED";
        #endif // and if gluings previously set. 
       newPens[ p0 ]->joinTo( (j+k)%5, newPens[ p1 ], tetInc.inverse()*NPerm5(j, (j+k)%5) ); 
      }
    }
   // we need to run through the triangles (equiv edges) of the pentachoron and
   // perform all of the appropriate gluings between tetrahedra.
   // step (a) check if it has ideal vertices.  If not, we are done! 

   for (unsigned long j=0; j<10; j++)
    {
      NPerm5 triInc( aPen->getTriangleMapping( j ) );
      const Dim4Triangle* aTri( aPen->getTriangle( j ) );
      // triInc[0 1 2] are the triangle vertices, [3 4] the vertices of the
      //  opposite edge. So we are gluing all the facets of tet's 3 and 4 which
      //  are incident to the triangle 012. 
      NPerm5 tet0inc( aPen->getTetrahedronMapping( triInc[3] ) );
      NPerm5 tet1inc( aPen->getTetrahedronMapping( triInc[4] ) );
      unsigned long tri0idx( tet0inc.preImageOf( triInc[4] ) );
      unsigned long tri1idx( tet1inc.preImageOf( triInc[3] ) );  
      const Dim4Tetrahedron* tet0( aPen->getTetrahedron( triInc[3] ) );
      const Dim4Tetrahedron* tet1( aPen->getTetrahedron( triInc[4] ) );
      NPerm5 tri0inc( tet0->getTriangleMapping( tri0idx ) );
      NPerm5 tri1inc( tet1->getTriangleMapping( tri1idx ) );
      // check if the triangle has any ideal vertices
      bool tIv(false);
      for (unsigned long k=0; k<3; k++) if (aTri->getVertex(k)->isIdeal())
        { tIv=true; break; }     
      // if tIv true, both sides are type CCt.  
      bool s0it(false), s1it(false); // check if adjacent tets have ideal vertices
      for (unsigned long k=0; k<4; k++)
        {
         if (aPen->getVertex(tet0inc[k])->isIdeal()) s0it=true;
         if (aPen->getVertex(tet1inc[k])->isIdeal()) s1it=true;
        }
      subDivNot p0( s0it ? _CCt : _CT, i, triInc[3], s0it ? tri0idx : 0 ); 
      subDivNot p1( s1it ? _CCt : _CT, i, triInc[4], s1it ? tri1idx : 0 ); 
        #ifdef DEBUG // test to check if p0 and p1 exist 
        if (newPens.find(p0)==newPens.end()) std::cerr<<"idealToFinite (11) p0 DNE";
        if (newPens.find(p1)==newPens.end()) std::cerr<<"idealToFinite (11) p1 DNE";
        if (newPens[p0]->adjacentPentachoron( s0it ? tri0idx : triInc[4] )!=NULL)
         std::cerr<<"idealToFinite (11) p0 GLUED";
        NPerm5 TP( (s1it ? tet1inc : NPerm5()).inverse()*
                    NPerm5(triInc[3], triInc[4])*(s0it ? tet0inc : NPerm5()) );
        if (newPens[p1]->adjacentPentachoron( TP[s0it ? tri0idx : triInc[4]]  
                    )!=NULL)
         std::cerr<<"idealToFinite (11) p1 GLUED";
        #endif // and if gluings previously set. 
      newPens[ p0 ]->joinTo( s0it ? tri0idx : triInc[4] , newPens[ p1 ], 
        (s1it ? tet1inc : NPerm5()).inverse()*NPerm5(triInc[3], 
                triInc[4])*(s0it ? tet0inc : NPerm5()));
      if (!tIv) continue; 
      for (unsigned long k=0; k<3; k++) if (aTri->getVertex(k)->isIdeal())
        { // _CCdt uses triangle coordinates, which are fixed.  
         p0.penType =_CCdt; p1.penType = _CCdt;
         p0.vtxIdx  = k;    p1.vtxIdx  = k;
    #ifdef DEBUG // test to check if p0 and p1 exist 
    if (newPens.find(p0)==newPens.end()) std::cerr<<"idealToFinite (12) p0 DNE";
    if (newPens.find(p1)==newPens.end()) std::cerr<<"idealToFinite (12) p1 DNE";
    if (newPens[p0]->adjacentPentachoron( 3 )!=NULL)
     std::cerr<<"idealToFinite (12) p0 GLUED";
    if (newPens[p1]->adjacentPentachoron( 3 )!=NULL)
     std::cerr<<"idealToFinite (12) p1 GLUED";
    #endif // and if gluings previously set. 
         newPens[ p0 ]->joinTo( 3, newPens[ p1 ], NPerm5() );
        }
    } // j loop -- edges
  }
 swapContents( *newTri );
 delete newTri;
 return true;
}




} // namespace regina
