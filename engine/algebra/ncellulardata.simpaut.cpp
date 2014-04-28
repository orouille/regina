
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

#include "algebra/ncellulardata.h"
#include "algebra/ncellulardata.simpaut.h"
#include <algorithm>

namespace regina {

NSimplicialAutGrp::NSimplicialAutGrp(const NTriangulation &input)
{
 tri3 = &input; tri4 = NULL;
 std::list< NIsomorphism* > isoList;
 input.findAllIsomorphisms( input, isoList );
 for (std::list< NIsomorphism* >::iterator i=isoList.begin(); i!=isoList.end(); i++)
  {
   isoStruct tempI; tempI.germ.resize( input.getNumberOfSimplices() );
   for (unsigned long j=0; j<tempI.germ.size(); j++)
    { tempI.germ[j].pen = (*i)->simpImage(j);  
      tempI.germ[j].perm = ((*i)->facetPerm(j).S4Index()); }
   fullMap.push_back( tempI );
   delete (*i); // clean up
  }
 sort( fullMap.begin(), fullMap.end() ); // we will need to find elements in this
  // list so we need it sorted.
}

NSimplicialAutGrp::NSimplicialAutGrp(const Dim4Triangulation &input)
{
 tri4 = &input; tri3 = NULL;
 std::list< Dim4Isomorphism* > isoList;
 input.findAllIsomorphisms( input, isoList );
 for (std::list< Dim4Isomorphism* >::iterator i=isoList.begin(); i!=isoList.end(); i++)
  {
   isoStruct tempI; tempI.germ.resize( input.getNumberOfSimplices() );
   for (unsigned long j=0; j<tempI.germ.size(); j++)
    { tempI.germ[j].pen = (*i)->simpImage(j);  
      tempI.germ[j].perm = ((*i)->facetPerm(j).S5Index()); }
   fullMap.push_back( tempI );
   delete (*i); // clean up
  }
 sort( fullMap.begin(), fullMap.end() ); // make binary searchable
}

std::vector< int > NSimplicialAutGrp::orientationAction() const
{
 std::vector< int > retval( fullMap.size() );
 for (unsigned long i=0; i<retval.size(); i++)
  {
  if (tri3) retval[i] = tri3->getTetrahedron(0)->orientation()*
                        NPerm4::S4[ fullMap[i].germ[0].perm ].sign()*
                        tri3->getTetrahedron( fullMap[i].germ[0].pen )->orientation();
  if (tri4) retval[i] = tri4->getPentachoron(0)->orientation()*
                        NPerm5::S5[ fullMap[i].germ[0].perm ].sign()*
                        tri4->getPentachoron( fullMap[i].germ[0].pen )->orientation();
  }
 return retval;
}


std::vector<NMatrixInt*> NSimplicialAutGrp::homologyH1action() const
{
 std::vector<NMatrixInt*> retval;
 NCellularData* cDat;
 if (tri3) cDat = new NCellularData(*tri3);
      else cDat = new NCellularData(*tri4);
 const NMarkedAbelianGroup* H1P( cDat->markedGroup( NCellularData::GroupLocator( 
    1, NCellularData::coVariant, NCellularData::STD_coord, 0) ) );

 if ( (H1P->getRank()==1) && (H1P->getNumberOfInvariantFactors()==0) )
  {
    retval.resize( fullMap.size() );
    for (unsigned long i=0; i<retval.size(); i++) 
     {
        // now we fill out the matrix.
        NMatrixInt mat( H1P->getRankCC(), H1P->getRankCC() );
        // using STD_coord. these are indexed by nicIx followed by icIx.
        // so computing the H1 CC map is going to be relatively straightforward. 
        unsigned long cellCount( cDat->cellCount( 
            NCellularData::ChainComplexLocator( 1, NCellularData::STD_coord ) ) );
        unsigned long edgCount( (tri3 ? tri3->getNumberOfEdges() :
            tri4->getNumberOfEdges() ) );
        unsigned long idCells( cellCount - edgCount );
        for (unsigned long j=0; j<edgCount; j++)
         {
          // step 1: figure out which edge the j-th edge is sent to. 
          //  we can do that by finding a top-simplex its in, map it, and
          //  check where the vertices go.  We then compute what edge that is...

          if (tri3) { // 3-dimensional case
           NPerm4 per3( tri3->getEdge(j)->getEmbedding(0).getVertices() );
           unsigned long tetnum3( tri3->simplexIndex( 
                        tri3->getEdge(j)->getEmbedding(0).getTetrahedron() ) );
           unsigned long tPen( fullMap[i].germ[ tetnum3 ].pen ); // target pen
           NPerm4 mPerm( NPerm4::S4[ fullMap[i].germ[ tetnum3 ].perm ] );
           // edge number in tet tPen. 
           unsigned long eNum( NEdge::edgeNumber[ mPerm[per3[0]] ][ mPerm[per3[1]] ] );
           NPerm4 eOr( tri3->getSimplex( tPen )->getEdgeMapping(eNum) );
           // now we need to find this edge Pen::getEdgeMapping class.  
           mat.entry( cDat->nicIxLookup( tri3->getSimplex(tPen)->getEdge(eNum) ), j ) 
                = ( (eOr[0] == mPerm[per3[0]]) ? 1 : -1 );
           } else { // 4-dimensional case
           NPerm5 per4( tri4->getEdge(j)->getEmbedding(0).getVertices() );
           unsigned long tetnum4( tri4->simplexIndex( 
                        tri4->getEdge(j)->getEmbedding(0).getPentachoron() ) );
           unsigned long tPen( fullMap[i].germ[ tetnum4 ].pen ); // target pen
           NPerm5 mPerm( NPerm5::S5[ fullMap[i].germ[ tetnum4 ].perm ] );
           // edge number in tet tPen. 
           unsigned long eNum( Dim4Edge::edgeNumber[ mPerm[per4[0]] ][ mPerm[per4[1]] ] );
           NPerm5 eOr( tri4->getSimplex( tPen )->getEdgeMapping(eNum) );
           // now we need to find this edge, Pen::getEdgeMapping class.  
           mat.entry( cDat->nicIxLookup( tri4->getSimplex(tPen)->getEdge(eNum) ), j ) 
                = ( (eOr[0] == mPerm[per4[0]]) ? 1 : -1 );
          }

         }
        for (unsigned long j=0; j<idCells; j++)
         { // the ideal parts
          // step 1: figure out which ideal edge the j-th ideal edge is sent to
          unsigned long INDX; // row index for mat, j is the col index
          int SIG; // sign of matrix at this entry

          if (tri3) { // 3-dim
            NPerm4 per3( tri3->getTriangle( 
                         cDat->icIndex(1,j).first )->getEmbedding(0).getVertices() );
            unsigned long tetnum3( tri3->simplexIndex( 
                tri3->getTriangle( cDat->icIndex(1,j).first )->getEmbedding(0).getTetrahedron() ) );
            unsigned long tPen( fullMap[i].germ[ tetnum3 ].pen );
            NPerm4 mPerm( NPerm4::S4[ fullMap[i].germ[ tetnum3 ].perm ] );
            unsigned long tNum( mPerm[ per3[ 3 ] ] );
            NPerm4 tOr( tri3->getSimplex( tPen )->getTriangleMapping( tNum ) );
            NPerm4 tOor( tOr.inverse()*mPerm*per3 ); // sign 
            SIG = tOor.sign();

            INDX = cDat->icIxLookup( tri3->getSimplex( tPen )->getTriangle(tNum), 
                                     tOor[cDat->icIndex(1,j).second] );
            } else { // 4-dim
            NPerm5 per4( tri4->getTriangle( 
                         cDat->icIndex(1,j).first )->getEmbedding(0).getVertices() );
            unsigned long tetnum4( tri4->simplexIndex( 
                tri4->getTriangle( cDat->icIndex(1,j).first )->getEmbedding(0).getPentachoron() ) );
            unsigned long tPen( fullMap[i].germ[ tetnum4 ].pen );
            NPerm5 mPerm( NPerm5::S5[ fullMap[i].germ[ tetnum4 ].perm ] );
            unsigned long tNum( Dim4Triangle::triangleNumber[ mPerm[ per4[0] ] ]
                                   [ mPerm[ per4[1] ] ][ mPerm[ per4[2] ] ] );
            NPerm5 tOr( tri4->getSimplex( tPen )->getTriangleMapping( tNum ) );
            NPerm5 tOor( tOr.inverse()*mPerm*per4 ); // we can recover the sign from this
                 // once we re-cast it to an NPerm3. 
            NPerm3 tOOr( tOor[0], tOor[1], tOor[2] ); // TODO: check this is okay...
            SIG = tOOr.sign(); 
            INDX = cDat->icIxLookup( tri4->getSimplex( tPen )->getTriangle(tNum), 
                            tOor[cDat->icIndex(1,j).second] );
          }
          mat.entry( edgCount + INDX, edgCount + j ) = SIG;
         }
 
        NHomMarkedAbelianGroup hom( *H1P, *H1P, mat );
        // basic checks isChainMap, isCycleMap, isIso
        if (!hom.isCycleMap()) { std::cout<<"ERROR! isCycleMap."<<std::endl; exit(1); }
        //if (!hom.isChainMap()) { std::cout<<"ERROR! isChainMap."<<std::endl; exit(1); }
        if (!hom.isIso())      { std::cout<<"ERROR! isIso."<<std::endl; exit(1); }
        // put matrix associated to hom into retval.
        retval[i] = new NMatrixInt( hom.getReducedMatrix() );
     }    
  }

 delete cDat;
 return retval;
}

// TODO: update to allow for cohomology
// TODO: perhaps allow coordinate systems other than STD_coord?  why? 
std::vector<NHomMarkedAbelianGroup*> NSimplicialAutGrp::homologyAction( 
        const NCellularData::GroupLocator &gloc ) const
{
 std::vector< NHomMarkedAbelianGroup* > retval;
 // initial tests
 if (gloc.hcs != NCellularData::STD_coord) return retval; // only STD_coord implemented
 if (gloc.dim > (tri3 ? 3 : 4)) return retval;

 NCellularData* cDat;
 if (tri3) cDat = new NCellularData(*tri3);
      else cDat = new NCellularData(*tri4);
 const NMarkedAbelianGroup* HP( cDat->markedGroup( gloc ) );
 if (!HP) return retval;
 retval.resize( fullMap.size() );
 unsigned long stdCount( cDat->stdCellCount( gloc.dim ) );
 unsigned long idCount( cDat->idCellCount( gloc.dim ) );
 unsigned long i;   // need to compute these 
 signed long SIG; // for each j.

 for (unsigned long A=0; A<retval.size(); A++) // automorphism A
  {
   NMatrixInt mat( HP->getRankCC(), HP->getRankCC() );
   for (unsigned long j=0; j<stdCount; j++)
    { // mat entry (?, j) std cells
      // find the j-th cell, get its embedding in a top-simplex.
      unsigned long domFac, domSim; // facet and simplex index
      unsigned long ranSim; // range simplex index 
      NPerm4 mPerm4, domPerm4; NPerm5 mPerm5, domPerm5;
      domFac = cDat->nicIndex( gloc.dim, j );
      if (tri3) switch ( gloc.dim )
      {
       case 0: 
         domSim = cDat->nicIxLookup( tri3->getVertex( domFac )->getEmbedding(0).getTetrahedron() );
         domPerm4 = tri3->getVertex( domFac )->getEmbedding(0).getVertices();
         ranSim = fullMap[A].germ[ domSim ].pen; // target simplex
         mPerm4 = NPerm4::S4[ fullMap[A].germ[ domSim ].perm ]; // permutation

         i = cDat->nicIxLookup( tri3->getSimplex( ranSim )->getVertex( mPerm4[ domPerm4[0] ] ) );
         SIG = 1; 
        break;
       case 1: {
         domSim = cDat->nicIxLookup( tri3->getEdge( domFac )->getEmbedding(0).getTetrahedron() );
         domPerm4 = tri3->getEdge( domFac )->getEmbedding(0).getVertices();
         ranSim = fullMap[A].germ[ domSim ].pen; // target simplex
         mPerm4 = NPerm4::S4[ fullMap[A].germ[ domSim ].perm ]; // permutation

         unsigned long eNum( NEdge::edgeNumber[mPerm4[domPerm4[0]]][mPerm4[domPerm4[1]]] );
         NPerm4 ranPerm4( tri3->getSimplex( ranSim )->getEdgeMapping( eNum ) );
         i = cDat->nicIxLookup( tri3->getSimplex( ranSim )->getEdge(eNum) );
         SIG = ( ( ranPerm4[0] == mPerm4[domPerm4[0]] ) ? 1 : -1 );
       } break;
       case 2: {
         domSim = cDat->nicIxLookup( tri3->getTriangle( domFac )->getEmbedding(0).getTetrahedron() );
         domPerm4 = tri3->getTriangle( domFac )->getEmbedding(0).getVertices();
         ranSim = fullMap[A].germ[ domSim ].pen; // target simplex
         mPerm4 = NPerm4::S4[ fullMap[A].germ[ domSim ].perm ]; // permutation

         unsigned long tNum( mPerm4[domPerm4[3]] );
         NPerm4 ranPerm4( tri3->getSimplex( ranSim )->getTriangleMapping( tNum ) );
         NPerm3 relPerm( ranPerm4.preImageOf( mPerm4[domPerm4[0]] ), 
                         ranPerm4.preImageOf( mPerm4[domPerm4[1]] ), 
                         ranPerm4.preImageOf( mPerm4[domPerm4[2]] ) );
         i = cDat->nicIxLookup( tri3->getSimplex( ranSim )->getTriangle(tNum) );
         SIG = relPerm.sign();
       } break;
       case 3: {
         domSim = cDat->nicIxLookup( tri3->getSimplex( domFac ) ); // domFac == domSim no?
         ranSim = fullMap[A].germ[ domSim ].pen; // target simplex
         mPerm4 = NPerm4::S4[ fullMap[A].germ[ domSim ].perm ]; // permutation
         i = cDat->nicIxLookup( tri3->getSimplex( ranSim ) );
         SIG = mPerm4.sign();
       } break;
      } else switch (gloc.dim) // 4-dimensional case
      {
       case 0: 
         domSim = cDat->nicIxLookup( tri4->getVertex( domFac )->getEmbedding(0).getPentachoron() );
         domPerm5 = tri4->getVertex( domFac )->getEmbedding(0).getVertices();
         ranSim = fullMap[A].germ[ domSim ].pen; // target simplex
         mPerm5 = NPerm5::S5[ fullMap[A].germ[ domSim ].perm ]; // permutation

         i = cDat->nicIxLookup( tri4->getSimplex( ranSim )->getVertex( mPerm5[ domPerm5[0] ] ) );
         SIG = 1; 
        break;
       case 1: {
         domSim = cDat->nicIxLookup( tri4->getEdge( domFac )->getEmbedding(0).getPentachoron() );
         domPerm5 = tri4->getEdge( domFac )->getEmbedding(0).getVertices();
         ranSim = fullMap[A].germ[ domSim ].pen; // target simplex
         mPerm5 = NPerm5::S5[ fullMap[A].germ[ domSim ].perm ]; // permutation

         unsigned long eNum( Dim4Edge::edgeNumber[mPerm5[domPerm5[0]]][mPerm5[domPerm5[1]]] );
         NPerm5 ranPerm5( tri4->getSimplex( ranSim )->getEdgeMapping( eNum ) );
         i = cDat->nicIxLookup( tri4->getSimplex( ranSim )->getEdge(eNum) );
         SIG = ( ( ranPerm5[0] == mPerm5[domPerm5[0]] ) ? 1 : -1 );
       } break;
       case 2: { 
         domSim = cDat->nicIxLookup( tri4->getTriangle( domFac )->getEmbedding(0).getPentachoron() );
         domPerm5 = tri4->getTriangle( domFac )->getEmbedding(0).getVertices();
         ranSim = fullMap[A].germ[ domSim ].pen; // target simplex
         mPerm5 = NPerm5::S5[ fullMap[A].germ[ domSim ].perm ]; // permutation

         unsigned long tNum( Dim4Triangle::triangleNumber[
            mPerm5[domPerm5[0]]][mPerm5[domPerm5[1]]][mPerm5[domPerm5[2]]] );
         NPerm5 ranPerm5( tri4->getSimplex( ranSim )->getTriangleMapping( tNum ) );
         NPerm3 relPerm( ranPerm5.preImageOf( mPerm5[domPerm5[0]] ), 
                         ranPerm5.preImageOf( mPerm5[domPerm5[1]] ), 
                         ranPerm5.preImageOf( mPerm5[domPerm5[2]] ) );
         i = cDat->nicIxLookup( tri4->getSimplex( ranSim )->getTriangle(tNum) );
         SIG = relPerm.sign();
       } break;
       case 3: { 
         domSim = cDat->nicIxLookup( tri4->getTetrahedron( domFac )->getEmbedding(0).getPentachoron() );
         domPerm5 = tri4->getTetrahedron( domFac )->getEmbedding(0).getVertices();
         ranSim = fullMap[A].germ[ domSim ].pen; // target simplex
         mPerm5 = NPerm5::S5[ fullMap[A].germ[ domSim ].perm ]; // permutation

         unsigned long tNum( mPerm5[domPerm5[4]] );
         NPerm5 ranPerm5( tri4->getSimplex( ranSim )->getTetrahedronMapping( tNum ) );
         NPerm4 relPerm( ranPerm5.preImageOf( mPerm5[domPerm5[0]] ), 
                         ranPerm5.preImageOf( mPerm5[domPerm5[1]] ), 
                         ranPerm5.preImageOf( mPerm5[domPerm5[2]] ),
                         ranPerm5.preImageOf( mPerm5[domPerm5[3]] ) );
         i = cDat->nicIxLookup( tri4->getSimplex( ranSim )->getTetrahedron(tNum) );
         SIG = relPerm.sign();
       } break;
       case 4: { 
         domSim = cDat->nicIxLookup( tri4->getSimplex( domFac ) );
         ranSim = fullMap[A].germ[ domSim ].pen; // target simplex
         mPerm5 = NPerm5::S5[ fullMap[A].germ[ domSim ].perm ]; // permutation

         i = cDat->nicIxLookup( tri4->getSimplex( ranSim ) );
         SIG = mPerm5.sign();
       } break;
      }
      mat.entry( i, j ) = SIG;
    }
   //            *********** Ideal cells below ***********
   for (unsigned long j=0; j<idCount; j++)
    { // mat entry (?, j) id cells
      // find the j-th ideal cell, get its embedding into a top-simplex

      unsigned long domFac, domFacV, domSim; // facet and simplex index
      unsigned long ranSim; // range simplex index 
      NPerm4 mPerm4, domPerm4; NPerm5 mPerm5, domPerm5;
      domFac = cDat->icIndex( gloc.dim, j ).first;
      domFacV = cDat->icIndex( gloc.dim, j ).second;

      if (tri3) switch ( gloc.dim )
      {
       case 0: { 
        domSim = cDat->nicIxLookup( tri3->getEdge( domFac )->getEmbedding(0).getTetrahedron() );
        domPerm4 = tri3->getEdge( domFac )->getEmbedding(0).getVertices();
        ranSim = fullMap[A].germ[ domSim ].pen;
        mPerm4 = NPerm4::S4[ fullMap[A].germ[ domSim ].perm ]; // permutation

        unsigned long eNum( NEdge::edgeNumber[ mPerm4[domPerm4[domFacV]] ]
                                             [ mPerm4[domPerm4[(domFacV+1) % 2]] ] );

        NPerm4 ranPerm4( tri3->getSimplex( ranSim )->getEdgeMapping( eNum ) );
        i = cDat->icIxLookup( tri3->getSimplex( ranSim )->getEdge( eNum ), 
             ranPerm4.preImageOf( mPerm4[domPerm4[domFacV]] ) );
        SIG = ( (ranPerm4[0] == mPerm4[domPerm4[0]]) ? 1 : -1 );
     } break;
       case 1: {
        domSim = cDat->nicIxLookup( tri3->getTriangle( domFac )->getEmbedding(0).getTetrahedron() );
        domPerm4 = tri3->getTriangle( domFac )->getEmbedding(0).getVertices();
        ranSim = fullMap[A].germ[ domSim ].pen;
        mPerm4 = NPerm4::S4[ fullMap[A].germ[ domSim ].perm ]; // permutation

        unsigned long tNum( mPerm4[domPerm4[3]] );
        NPerm4 ranPerm4( tri3->getSimplex( ranSim )->getTriangleMapping( tNum ) );
        NPerm3 tPara( ranPerm4.preImageOf( mPerm4[domPerm4[0]] ), 
                      ranPerm4.preImageOf( mPerm4[domPerm4[1]] ),
                      ranPerm4.preImageOf( mPerm4[domPerm4[2]] ) );
        i = cDat->icIxLookup( tri3->getSimplex( ranSim )->getTriangle( tNum ), 
                   tPara[ domFacV ]);
        SIG = tPara.sign();
     } break;
       case 2: {
        domSim = cDat->nicIxLookup( tri3->getTetrahedron( domFac ) );
        ranSim = fullMap[A].germ[ domSim ].pen;
        mPerm4 = NPerm4::S4[ fullMap[A].germ[ domSim ].perm ]; // permutation

        i = cDat->icIxLookup( tri3->getSimplex( ranSim ), mPerm4[domFacV] );
        SIG = mPerm4.sign();
     } break;
      } else switch ( gloc.dim ) // 4-dimensions
      {
       case 0: {
        domSim = cDat->nicIxLookup( tri4->getEdge( domFac )->getEmbedding(0).getPentachoron() );
        domPerm5 = tri4->getEdge( domFac )->getEmbedding(0).getVertices();
        ranSim = fullMap[A].germ[ domSim ].pen;
        mPerm5 = NPerm5::S5[ fullMap[A].germ[ domSim ].perm ]; // permutation

        unsigned long eNum( Dim4Edge::edgeNumber[ mPerm5[domPerm5[domFacV]] ]
                                             [ mPerm5[domPerm5[(domFacV+1) % 2]] ] );

        NPerm5 ranPerm5( tri4->getSimplex( ranSim )->getEdgeMapping( eNum ) );
        i = cDat->icIxLookup( tri4->getSimplex( ranSim )->getEdge( eNum ), 
             ranPerm5.preImageOf( mPerm5[domPerm5[domFacV]] ) );
        SIG = ( (ranPerm5[0] == mPerm5[domPerm5[0]]) ? 1 : -1 );
     } break;
       case 1: {
        domSim = cDat->nicIxLookup( tri4->getTriangle( domFac )->getEmbedding(0).getPentachoron() );
        domPerm5 = tri4->getTriangle( domFac )->getEmbedding(0).getVertices();
        ranSim = fullMap[A].germ[ domSim ].pen;
        mPerm5 = NPerm5::S5[ fullMap[A].germ[ domSim ].perm ]; // permutation

        unsigned long tNum( Dim4Triangle::triangleNumber[mPerm5[domPerm5[domFacV]]]
           [mPerm5[domPerm5[(domFacV + 1) % 3]]][mPerm5[domPerm5[(domFacV + 2) % 3]]] );
        NPerm5 ranPerm5( tri4->getSimplex( ranSim )->getTriangleMapping( tNum ) );
        NPerm3 tPara( ranPerm5.preImageOf( mPerm5[domPerm5[0]] ), 
                      ranPerm5.preImageOf( mPerm5[domPerm5[1]] ),
                      ranPerm5.preImageOf( mPerm5[domPerm5[2]] ) );
        i = cDat->icIxLookup( tri4->getSimplex( ranSim )->getTriangle( tNum ), 
                   tPara[ domFacV ]);
        SIG = tPara.sign();
     } break;
       case 2: {
        domSim = cDat->nicIxLookup( tri4->getTetrahedron( domFac )->getEmbedding(0).getPentachoron() );
        domPerm5 = tri4->getTetrahedron( domFac )->getEmbedding(0).getVertices();
        ranSim = fullMap[A].germ[ domSim ].pen;
        mPerm5 = NPerm5::S5[ fullMap[A].germ[ domSim ].perm ]; // permutation

        unsigned long tNum( mPerm5[domPerm5[4]] );
        NPerm5 ranPerm5( tri4->getSimplex( ranSim )->getTetrahedronMapping( tNum ) );
        NPerm4 tPara( ranPerm5.preImageOf( mPerm5[domPerm5[0]] ),
                      ranPerm5.preImageOf( mPerm5[domPerm5[1]] ),
                      ranPerm5.preImageOf( mPerm5[domPerm5[2]] ),
                      ranPerm5.preImageOf( mPerm5[domPerm5[3]] ) );
        i = cDat->icIxLookup( tri4->getSimplex( ranSim )->getTetrahedron( tNum ), 
                    tPara[ domFacV ] );
        SIG = tPara.sign();
     } break;
       case 3: {
        domSim = cDat->nicIxLookup( tri4->getPentachoron( domFac ) );
        ranSim = fullMap[A].germ[ domSim ].pen;
        mPerm5 = NPerm5::S5[ fullMap[A].germ[ domSim ].perm ]; // permutation

        i = cDat->icIxLookup( tri4->getSimplex( ranSim ), mPerm5[domFacV] );
        SIG = mPerm5.sign();
     } break;
      }

      mat.entry( i+stdCount, j+stdCount ) = SIG;
    }

   retval[A] = new NHomMarkedAbelianGroup( *HP, *HP, mat );
   if (!retval[A]->isCycleMap()) { 
     std::cout<<"NSimplicialAutGrp::homologyAction() ERROR! isCycleMap()."<<
     std::endl; exit(1); }
   if (!retval[A]->isIso())      { 
     std::cout<<"NSimplicialAutGrp::homologyAction() ERROR! isIso()."<<
     std::endl; exit(1); }
  }

 delete HP;
 return retval;
}


bool centroid::operator<(const centroid &rhs) const
 { 
  std::set<unsigned long>::const_iterator j=rhs.points.begin();
  for (std::set< unsigned long >::const_iterator i=points.begin(); 
   ( (i!=points.end()) && (j!=rhs.points.end()) ); i++, j++)
  {
    if ((*i) < (*j)) return true;
    if ((*i) > (*j)) return false;
  } 
 if (j!=rhs.points.end()) return true;
 return false;
 }
 

std::set< linearFacet > linearFacet::bdryFacets(const NTriangulation *tri3, 
                                            const Dim4Triangulation* tri4) const
{
 std::set< linearFacet > retval;
 if ((!tri3) && (!tri4)) { std::cout<<"linearFacet::bdryFacets ERROR 1."<<std::endl; exit(1); }
 // step 1: copy of this object with single vertex deleted for all vertices. 
 if ( (dim==0) || (sdim==0) ) return std::set< linearFacet >();
 // basic sanity checks
 if ( (tri3 && (dim>3)) || (tri4 && (dim>4)) || (tri3 && (sdim>3)) ||
      (tri4 && (sdim>4)) || (dim > sdim) ) return std::set< linearFacet >();
 NCellularData *cDat;
 if (tri3) cDat = new NCellularData(*tri3); 
 else      cDat = new NCellularData(*tri4);
 // counts of cells in dimension sdim. 
 unsigned long stdCount( cDat->stdCellCount( sdim ) );
 unsigned long idCount( cDat->idCellCount( sdim ) );
 for (std::set< centroid >::const_iterator i=vCentres.begin(); i!=vCentres.end(); i++)
  { // make a copy of this linearFacet with *i deleted.
    linearFacet Bi( *this );
    Bi.vCentres.erase(*i);
    std::set< unsigned long > VU( Bi.verticesUsed() ); // vertices in the facet
    std::set< unsigned long > VuU( Bi.verticesUnUsed() ); // vertices out of the facet
    if (sindx >= stdCount) { std::cout<<"linearFacet::bdryFacets ERROR 2."<<std::endl; exit(1); }
    // which boundary facet does this live in?  
    // sindx is its STD_coord, so there's non-ideal, ideal and tri3 and tri4 possibilities.
    switch (sdim) {
     case 1: { // can only be a standard edge 
      if (tri3) { // the end is the remaining single element of VU
       if (VU.size() != 1) { std::cout<<"linearFacet::bdryFacets ERROR 3."<<std::endl; exit(1); }
        const NEdge* edg( tri3->getEdge( cDat->nicIndex( sdim, sindx ) ) );
        unsigned long I( *VU.begin() );
        if ( !edg->getVertex(I)->isIdeal() ) 
         { retval.insert( linearFacet( 0, 0, cDat->nicIxLookup( edg->getVertex(I) ) ) ); }
       } else 
       { // tri4
        if (VU.size() != 1) { std::cout<<"linearFacet::bdryFacets ERROR 4."<<std::endl; exit(1); }
        const Dim4Edge* edg( tri4->getEdge( cDat->nicIndex( sdim, sindx ) ) );
        unsigned long I( *VU.begin() );
        if ( !edg->getVertex(I)->isIdeal() ) 
         { retval.insert( linearFacet( 0, 0, cDat->nicIxLookup( edg->getVertex(I) ) ) ); }
       }
      } break;
     case 2: // sdim==2, can be standard tri, or edge bisecting tri
      if (tri3) {
       const NTriangle* tri( tri3->getTriangle( cDat->nicIndex( sdim, sindx ) ) ); 
       if (vCentres.size()==2) 
        { // edge bisecting a tri.
        if (VuU.size() == 1) { // need edg with {0,1} label
         linearFacet temp( 0, 1, cDat->nicIxLookup( tri->getEdge( *VuU.begin() ) ) );
         std::set< unsigned long > T01; T01.insert(0); T01.insert(1);  temp.vCentres.insert( centroid( T01 ) );
         retval.insert( temp );
         }
        else { // VuU.size()==2, VU.size()==1, two cases, ideal or not. 
         if (!tri->getVertex( *VU.begin() )->isIdeal())
          { 
           linearFacet temp( 0, 0, cDat->nicIxLookup( tri->getVertex( *VU.begin() ) ) );
           retval.insert(temp);
          }
         }
        } // end edge bisecting triangle 
       else
        { // standard triangle.  Standard edge boundaries. VuU.size()==1, the opp vtx.
         linearFacet temp( 1, 1, cDat->nicIxLookup( tri->getEdge( *VuU.begin() ) ) );
         std::set< unsigned long > T0; T0.insert(0); temp.vCentres.insert( centroid( T0 ) ); 
         T0.clear(); T0.insert(1); temp.vCentres.insert( centroid( T0 ) );
         retval.insert( temp );
        }
       }
      else { // sdim == 2, dim4
       const Dim4Triangle* tri( tri4->getTriangle( cDat->nicIndex( sdim, sindx ) ) ); 
       if (vCentres.size()==2) { // edge bisecting a tri {0}, {1,2}
        if (VuU.size() == 1) { // need edg with {0,1} label
         linearFacet temp( 0, 1, cDat->nicIxLookup( tri->getEdge( *VuU.begin() ) ) );
         std::set< unsigned long > T01; T01.insert(0); T01.insert(1); 
         centroid Temp( T01 ); temp.vCentres.insert(Temp);
         retval.insert( temp );
        }
       else 
        { // VuU.size()==2, VU.size()==1, two cases, ideal or not. 
         if (!tri->getVertex( *VU.begin() )->isIdeal())
          {
           linearFacet temp( 0, 0, cDat->nicIxLookup( tri->getVertex( *VU.begin() ) ) );
           retval.insert(temp);
          }
        }
       } // end edge bisecting triangle 
      else
       { // standard triangle.  Standard edge boundaries. VuU.size()==1, the opp vtx.
        linearFacet temp( 1, 1, cDat->nicIxLookup( tri->getEdge( *VuU.begin() ) ) );
        std::set< unsigned long > T0; T0.insert(0); centroid Temp(T0); temp.vCentres.insert(Temp); 
        T0.clear(); T0.insert(1); centroid Temp1(T0); temp.vCentres.insert(Temp1);
        retval.insert( temp );
       }
      }
    break; 
     case 3: // in tetrahedron
      if (tri3) { // tri bisecting tet first, {0}, {1}, {2,3}, cyc.size()==3
       const NTetrahedron* tet(tri3->getTetrahedron( cDat->nicIndex( sdim, sindx ) ) );
       if (vCentres.size()==4) // whole tetrahedron
        {
         linearFacet temp( 2, 2, cDat->nicIxLookup( tet->getTriangle( *VuU.begin() ) ) );
         std::set< unsigned long > T0; T0.insert(0); temp.vCentres.insert( centroid(T0) ); 
         T0.clear(); T0.insert(1); temp.vCentres.insert( centroid(T0) );
         T0.clear(); T0.insert(2); temp.vCentres.insert( centroid(T0) );
         retval.insert( temp );
        } else
       if (vCentres.size()==3) // tri bisecting tet. 
        { // which boundary facet are we on?
         if ( i->points.size() == 1 ) // edge separating tri
          { 
           std::set< unsigned long > smallV, bigV;
           for (std::set< centroid >::const_iterator j=vCentres.begin(); j!=vCentres.end(); j++)
            { 
             if (j==i) continue;
             if (j->points.size()==1) smallV = j->points; else bigV = j->points;
            }
           linearFacet temp( 1, 2, cDat->nicIxLookup( tet->getTriangle( *VuU.begin() ) ) );
           NPerm4 iTri( tet->getTriangleMapping( *VuU.begin() ) );
           std::set< unsigned long > T0; T0.insert( iTri.preImageOf( *smallV.begin() ) ); temp.vCentres.insert( centroid(T0) );
           T0.clear(); T0.insert( iTri.preImageOf( *bigV.begin() ) ); 
                       T0.insert( iTri.preImageOf( *bigV.rbegin() ) ); temp.vCentres.insert( centroid(T0) );
           retval.insert(temp); 
          } else // entire edge
          { // whole edge.  Figure out which one it is. 
           std::set< unsigned long > smallV; 
           for (std::set< centroid >::const_iterator j=vCentres.begin(); j!=vCentres.end(); j++)
            {
             if (j==i) continue;
             smallV.insert( *j->points.begin() );
            }
           unsigned long eNum( NEdge::edgeNumber[*smallV.begin()][*smallV.rbegin()] ); 
           linearFacet temp( 1, 1, cDat->nicIxLookup( tet->getEdge( eNum ) ) ); // look up index...
           std::set< unsigned long > T0; T0.insert(0);  temp.vCentres.insert( centroid(T0) );
           T0.clear(); T0.insert(1); temp.vCentres.insert( centroid(T0) );
           retval.insert(temp);
          } 
        } else // vCentres has two elements. {{0,1},{2,3}} or {{0}, {1,2,3}}  
        { 
          if (VU.size() == VuU.size()) // 2 and 2
           { 
            unsigned long eNum( NEdge::edgeNumber[*VU.begin()][*VU.rbegin()] );
            linearFacet temp( 0, 1, cDat->nicIxLookup( tet->getEdge( eNum ) ) );
            std::set< unsigned long > T0; T0.insert(0); T0.insert(1); temp.vCentres.insert( centroid(T0) );
            retval.insert(temp);
           }
          else // 3 and 1
           {
            if (VU.size() == 1) { if (!tet->getVertex(*VU.begin())->isIdeal()) { // plain vertex
             linearFacet temp( 0, 0, cDat->nicIxLookup( tet->getVertex( *VU.begin() ) ) );
             retval.insert(temp);
            } } else
            { // centroid of triangle
             if (VU.size() != 3) { std::cout<<"linearFacets::bdryFacets ERROR ?"<<std::endl; exit(1); }
             linearFacet temp( 0, 2, cDat->nicIxLookup( tet->getTriangle( *VuU.begin() ) ) );
             std::set< unsigned long > T0; T0.insert(0); T0.insert(1); T0.insert(2);
             temp.vCentres.insert( centroid(T0) );
             retval.insert(temp);
            }
           }
        }
      }
     else 
      { // dim4, copy dim3 case above 
       const Dim4Tetrahedron* tet( tri4->getTetrahedron( cDat->nicIndex( sdim, sindx ) ) );
       if (vCentres.size()==4) // whole tetrahedron
        {
         linearFacet temp( 2, 2, cDat->nicIxLookup( tet->getTriangle( *VuU.begin() ) ) );
         std::set< unsigned long > T0; T0.insert(0); temp.vCentres.insert( centroid(T0) ); 
         T0.clear(); T0.insert(1); temp.vCentres.insert( centroid(T0) );
         T0.clear(); T0.insert(2); temp.vCentres.insert( centroid(T0) );
         retval.insert( temp );
        } else
       if (vCentres.size()==3) // tri bisecting tet. 
        { // which boundary facet are we on?
         if ( i->points.size() == 1 ) // edge separating tri
          { 
           std::set< unsigned long > smallV, bigV;
           for (std::set< centroid >::const_iterator j=vCentres.begin(); j!=vCentres.end(); j++)
            { 
             if (j==i) continue;
             if (j->points.size()==1) smallV = j->points; else bigV = j->points;
            }
           linearFacet temp( 1, 2, cDat->nicIxLookup( tet->getTriangle( *VuU.begin() ) ) );
           NPerm5 iTri( tet->getTriangleMapping( *VuU.begin() ) );
           std::set< unsigned long > T0; T0.insert( iTri.preImageOf( *smallV.begin() ) ); temp.vCentres.insert( centroid(T0) );
           T0.clear(); T0.insert( iTri.preImageOf( *bigV.begin() ) ); 
                       T0.insert( iTri.preImageOf( *bigV.rbegin() ) ); temp.vCentres.insert( centroid(T0) );
           retval.insert(temp); 
          } else // entire edge
          { // whole edge.  Figure out which one it is. 
           std::set< unsigned long > smallV; 
           for (std::set< centroid >::const_iterator j=vCentres.begin(); j!=vCentres.end(); j++)
            {
             if (j==i) continue;
             smallV.insert( *j->points.begin() );
            }
           unsigned long eNum( NEdge::edgeNumber[*smallV.begin()][*smallV.rbegin()] ); 
           linearFacet temp( 1, 1, cDat->nicIxLookup( tet->getEdge( eNum ) ) ); // look up index...
           std::set< unsigned long > T0; T0.insert(0);  temp.vCentres.insert( centroid(T0) );
           T0.clear(); T0.insert(1); temp.vCentres.insert( centroid(T0) );
           retval.insert(temp);
          } 
        } else // vCentres has two elements. {{0,1},{2,3}} or {{0}, {1,2,3}}  
        { 
          if (VU.size() == VuU.size()) // 2 and 2
           { 
            unsigned long eNum( NEdge::edgeNumber[*VU.begin()][*VU.rbegin()] );
            linearFacet temp( 0, 1, cDat->nicIxLookup( tet->getEdge( eNum ) ) );
            std::set< unsigned long > T0; T0.insert(0); T0.insert(1); temp.vCentres.insert( centroid(T0) );
            retval.insert(temp);
           }
          else // 3 and 1
           {
            if (VU.size() == 1) { if (!tet->getVertex(*VU.begin())->isIdeal()) { // plain vertex
             linearFacet temp( 0, 0, cDat->nicIxLookup( tet->getVertex( *VU.begin() ) ) );
             retval.insert(temp);
            } } else
            { // centroid of triangle
             if (VU.size() != 3) { std::cout<<"linearFacets::bdryFacets ERROR 5"<<std::endl; exit(1); }
             linearFacet temp( 0, 2, cDat->nicIxLookup( tet->getTriangle( *VuU.begin() ) ) );
             std::set< unsigned long > T0; T0.insert(0); T0.insert(1); T0.insert(2);
             temp.vCentres.insert( centroid(T0) );
             retval.insert(temp);
            }
           }
        }
      }
    break;
     case 4: 
      { // only a 4-dimensional case
        const Dim4Pentachoron* pen( tri4->getPentachoron( cDat->nicIndex( sdim, sindx ) ) );
        if (vCentres.size() == 5) // whole pentachoron
         {
            linearFacet temp( 3, 3, cDat->nicIxLookup( pen->getTetrahedron( *VuU.begin() ) ) );
            std::set< unsigned long > T0; T0.insert(0); temp.vCentres.insert( centroid(T0) );
            T0.clear(); T0.insert(1); temp.vCentres.insert( centroid(T0) );
            T0.clear(); T0.insert(2); temp.vCentres.insert( centroid(T0) );
            T0.clear(); T0.insert(3); temp.vCentres.insert( centroid(T0) );
            retval.insert(temp);
         } else
        if (vCentres.size() == 4) // tet separating pen
         {
          if (VuU.size()==2) // opp the edge, this is a full triangle, VU gives tri
           { 
            std::vector<unsigned long> TN;
            for (std::set< unsigned long >::iterator j=VU.begin(); j!=VU.end(); j++)
             TN.push_back(*j);
            unsigned long tNum( Dim4Triangle::triangleNumber[TN[0]][TN[1]][TN[2]] );
            linearFacet temp( 2, 2, cDat->nicIxLookup( pen->getTriangle( tNum ) ) ); 
            std::set< unsigned long > T0; T0.insert(0); temp.vCentres.insert( centroid(T0) );
            T0.clear(); T0.insert(1); temp.vCentres.insert( centroid(T0) );
            T0.clear(); T0.insert(2); temp.vCentres.insert( centroid(T0) );
            retval.insert(temp);                              
           }
          else // tri separating tet.  
           { 
            if (VuU.size()!=1) { std::cout<<"linearFacets::bdryFacets ERROR 6."<<std::endl; exit(1); }
            NPerm5 tEmb( pen->getTetrahedronMapping( *VuU.begin() ) );
            linearFacet temp( 2, 3, cDat->nicIxLookup( pen->getTetrahedron( *VuU.begin() ) ) );
            std::set< unsigned long > Pair, Sing1, Sing2;
            for (std::set< centroid >::const_iterator j=vCentres.begin(); j!=vCentres.end(); j++)
             {
              if (j==i) continue;
              if (j->points.size()==2) { Pair.insert( tEmb.preImageOf( *j->points.begin() ) );
                                         Pair.insert( tEmb.preImageOf( *j->points.rbegin() ) ); }
              else { if (Sing1.size()>0) Sing2.insert( tEmb.preImageOf( *j->points.begin() ) );
                     else Sing1.insert( tEmb.preImageOf( *j->points.begin() ) ); }
             }
            temp.vCentres.insert( centroid( Pair ) );
            temp.vCentres.insert( centroid( Sing1 ) );
            temp.vCentres.insert( centroid( Sing2 ) );
            retval.insert(temp);
           }
         } else
        if (vCentres.size() == 3) // 2 edge barycentres and 1 vtx, or tri barycentre and two vertices
         {
          unsigned long max(0);
          for (std::set< centroid >::const_iterator j=vCentres.begin(); j!=vCentres.end(); j++)
           if (j->points.size()>max) max = j->points.size();
          if (max == 3) // tri barycentre and two vertices
           {
            if (VuU.size()==1) { // tri barycentre and vertex
              NPerm5 tEmb( pen->getTetrahedronMapping( *VuU.begin() ) );
              linearFacet temp( 1, 3, cDat->nicIxLookup( pen->getTetrahedron( *VuU.begin() ) ) );
              std::set< unsigned long > Trip; 
              for (std::set< centroid >::const_iterator j=vCentres.begin(); j!=vCentres.end(); j++)
               {
                if (j==i) continue;
                if (j->points.size()==1) { Trip.insert( tEmb.preImageOf( *j->points.begin() ) );
                                           temp.vCentres.insert( centroid( Trip ) ); Trip.clear(); }
                else
                 {
                  for (std::set< unsigned long >::const_iterator k=j->points.begin(); k!=j->points.end(); k++)
                    Trip.insert( tEmb.preImageOf( *k ) );
                  temp.vCentres.insert( centroid( Trip ) ); Trip.clear(); 
                 }
               }
              retval.insert( temp );
             }
            else
             { // two vertices, i.e. proper edge
               unsigned long eNum( Dim4Edge::edgeNumber[*VU.begin()][*VU.rbegin()] );
               linearFacet temp( 1, 1, cDat->nicIxLookup( pen->getEdge( eNum ) ) );
               std::set< unsigned long > T0; T0.insert(0); temp.vCentres.insert( centroid(T0) );
               T0.clear(); T0.insert(1); temp.vCentres.insert( centroid(T0) );
               retval.insert(temp);
             }
           } else // 2 edge barycentres and 1 vtx
           {
            if (VuU.size()==1) 
             { // edge between two edge barycentres
               linearFacet temp( 1, 3, cDat->nicIxLookup( pen->getTetrahedron( *VuU.begin() ) ) );
               NPerm5 tInc( pen->getTetrahedronMapping( *VuU.begin() ) );
               std::set< unsigned long > Pair; 
               for (std::set< centroid >::const_iterator j=vCentres.begin(); j!=vCentres.end(); j++)
                {  
                  if (j==i) continue;
                  Pair.insert( tInc.preImageOf( *j->points.begin() ) );
                  Pair.insert( tInc.preImageOf( *j->points.rbegin() ) );
                  temp.vCentres.insert( centroid( Pair ) );
                }
               retval.insert(temp);
             } else
             { // edge between edge barycentre and vtx
                // which triangle is it?  VU gives it to us. 
                std::vector< unsigned long > T;
                for (std::set< unsigned long >::iterator j=VU.begin(); j!=VU.end(); j++)
                    T.push_back( *j );
                unsigned long tNum( Dim4Triangle::triangleNumber[T[0]][T[1]][T[2]] );
                NPerm5 tInc( pen->getTriangleMapping( tNum ) );
                linearFacet temp( 1, 2, cDat->nicIxLookup( pen->getTriangle( tNum ) ) );
                std::set< unsigned long > Pair, Vtx;
                for (std::set< centroid >::const_iterator j=vCentres.begin(); j!=vCentres.end(); j++)
                 {
                  if (j==i) continue;
                  if (j->points.size()==2) { Pair.insert( tInc.preImageOf( *j->points.begin() ) );
                                             Pair.insert( tInc.preImageOf( *j->points.rbegin() ) ); 
                                             temp.vCentres.insert( centroid( Pair ) ); }
                  else { Vtx.insert( tInc.preImageOf( *j->points.begin() ) ); 
                         temp.vCentres.insert( centroid( Vtx ) ); }
                 }
                retval.insert(temp);
             }
           }
         } 
        else // tri barycentre and edge bctr, or tet barycentre and vtx.
         {
          if (vCentres.size() != 2) { std::cout<<"linearFacets::bdryFacets ERROR 7."<<std::endl; exit(1); }
          unsigned long max(0);
          for (std::set< centroid >::const_iterator j=vCentres.begin(); j!=vCentres.end(); j++)
           if (j->points.size()>max) max = j->points.size();
          if (max==4) // tet and vtx
           {
            if (VuU.size()==1) // tet
             {
               linearFacet temp( 0, 3, cDat->nicIxLookup( pen->getTetrahedron( *VuU.begin() ) ) );
               std::set< unsigned long > T0; T0.insert(0); T0.insert(1); T0.insert(2); T0.insert(3);
               temp.vCentres.insert( centroid( T0 ) ); 
               retval.insert(temp);
             } else // vtx
             { 
               if (!pen->getVertex(*VU.begin())->isIdeal())
                {
                 linearFacet temp( 0, 0, cDat->nicIxLookup( pen->getVertex( *VU.begin()) ) );
                 retval.insert(temp);
               }
             }
           } else // tri ctr and edg ctr
           {
             if (VuU.size()==2) // tri
              {
               std::vector< unsigned long > V;
               for (std::set< unsigned long >::const_iterator j=VU.begin(); j!=VU.end(); j++)
                 V.push_back( *j );
               unsigned long tNum( Dim4Triangle::triangleNumber[V[0]][V[1]][V[2]] );           
               linearFacet temp( 0, 2, cDat->nicIxLookup( pen->getTriangle( tNum ) ) );
               std::set< unsigned long > T0; T0.insert(0); T0.insert(1); T0.insert(2);
               temp.vCentres.insert( centroid(T0) );
               retval.insert(temp);
              } else // edg
              {
               unsigned long eNum( Dim4Edge::edgeNumber[*VU.begin()][*VU.rbegin()] );
               linearFacet temp( 0, 1, cDat->nicIxLookup( pen->getEdge( eNum ) ) );
               std::set< unsigned long > T0; T0.insert(0); T0.insert(1);
               temp.vCentres.insert( centroid(T0) );
               retval.insert(temp);
              }
           }
         } 
     } break;
    }
   }

 delete cDat;
 return retval;
}

bool linearFacet::operator<(const linearFacet &rhs) const
{
 if (dim < rhs.dim) return true;
 if (dim > rhs.dim) return false;
 if (sdim < rhs.sdim) return true;
 if (sdim > rhs.sdim) return false;
 if (sindx < rhs.sindx) return true;
 if (sindx > rhs.sindx) return false;
 std::set< centroid >::const_iterator j=rhs.vCentres.begin();
 for (std::set< centroid >::const_iterator i=vCentres.begin(); 
  ( (i!=vCentres.end()) && (j!=rhs.vCentres.end()) ); i++, j++)
  {
    if ((*i) < (*j)) return true;
    if ((*j) < (*i)) return false;        
  }
 return false; // This appears to be all we need. 
}

//friend 
std::ostream& operator << (std::ostream& out, const linearFacet& p)
{
 out<<"[D"<<p.dim<<"S"<<p.sdim<<"I"<<p.sindx<<" ";
 for (std::set< centroid >::iterator i=p.vCentres.begin(); i!=p.vCentres.end(); i++)
  {
   if (i!=p.vCentres.begin()) out<<"-";
   for (std::set< unsigned long >::iterator j=i->points.begin(); j!=i->points.end(); j++)
    { out<<*j; }
  }
 out<<"]";
 return out;
}


std::set< unsigned long > linearFacet::verticesUsed() const
{
 std::set< unsigned long > retval;
  for (std::set< centroid >::const_iterator i=vCentres.begin(); i!=vCentres.end(); i++)
   for (std::set< unsigned long >::const_iterator j=i->points.begin(); j!=i->points.end(); j++)
     retval.insert( *j );
 return retval;
}

std::set< unsigned long > linearFacet::verticesUnUsed() const
{ 
 std::set< unsigned long > retval;
 for (unsigned long i=0; i<(sdim+1); i++)
   retval.insert(i);

 for (std::set< centroid >::const_iterator i=vCentres.begin(); i!=vCentres.end(); i++)
   for (std::set< unsigned long >::const_iterator j=i->points.begin(); j!=i->points.end(); j++)
     retval.erase( *j );

 return retval;
}


bool linearFacet::isValid() const
{
 //  (0) first check, given the dimension, check it has the right number of elements in cyc
 if (vCentres.size() != (dim+1)) return false;
 //  (1) If ideal, all ideal of the same type.  If not ideal, all not ideal. 

 for (std::set< centroid >::const_iterator i=vCentres.begin(); i!=vCentres.end(); i++)
  for (std::set< unsigned long >::const_iterator j=i->points.begin(); j!=i->points.end(); j++)
     // check index in bounds
     if ( (*j) >= (sdim+1) ) return false;
  
 if (sdim > 4) return false;
 if (dim > sdim) return false;

 //  (2) Check they form an appropriate partition. 
 //      if not ideal, then parition of all 0..sdim in the non-ideal case
 //      if ideal, partition of 0..sdim with idVrt removed. 
 std::set< unsigned long > pTest; //std::set< unsigned long >::iterator tI(pTest.begin());
 for (unsigned long i=0; i<(sdim+1); i++)
   pTest.insert(i);

 unsigned long count(0);
 for (std::set< centroid >::const_iterator i=vCentres.begin(); i!=vCentres.end(); i++)
  for (std::set< unsigned long >::const_iterator j=i->points.begin(); j!=i->points.end(); j++)
   {
    pTest.erase( (*j) );
    count++;
   }
 if (!pTest.empty()) return false;
 if (count != (sdim+1)) return false;
 
 return true;
}

bool linearFacet::isIdeal(const NTriangulation *tri3, const Dim4Triangulation* tri4) const
{
 NCellularData* cDat;
 bool retval(false);
 if (dim == 0) return false;
 if (tri3) cDat = new NCellularData(*tri3);
      else cDat = new NCellularData(*tri4);
 switch (sdim) {
    case 1: 
     for (std::set< centroid >::const_iterator i=vCentres.begin(); i!=vCentres.end(); i++)
      if (i->points.size()==1) if ( (tri3 ? tri3->getEdge( sindx )->getVertex( *i->points.begin() )->isIdeal() :
                                            tri4->getEdge( sindx )->getVertex( *i->points.begin() )->isIdeal() ) )
       retval=true;
     break;
    case 2: 
     for (std::set< centroid >::const_iterator i=vCentres.begin(); i!=vCentres.end(); i++)
      if (i->points.size()==1) if ( (tri3 ? tri3->getTriangle( sindx )->getVertex( *i->points.begin() )->isIdeal() :
                                            tri4->getTriangle( sindx )->getVertex( *i->points.begin() )->isIdeal() ) )
       retval=true;
     break;
    case 3: 
     for (std::set< centroid >::const_iterator i=vCentres.begin(); i!=vCentres.end(); i++)
      if (i->points.size()==1) if ( (tri3 ? tri3->getTetrahedron( sindx )->getVertex( *i->points.begin() )->isIdeal() :
                                            tri4->getTetrahedron( sindx )->getVertex( *i->points.begin() )->isIdeal() ) )
       retval=true;
     break;
    case 4: 
     for (std::set< centroid >::const_iterator i=vCentres.begin(); i!=vCentres.end(); i++)
      if (i->points.size()==1) if ( tri4->getPentachoron( sindx )->getVertex( *i->points.begin() )->isIdeal() ) 
       retval=true;
     break;
 } // end switch sdim statement
 delete cDat;
 return retval; // default
}


/* Given a simplicial automorphism of a simplicial complex, the fixed point set
 * consists of a union of:
 *  a) simplices from the simplicial complex itself
 *  b) subsets of simplices consisting of the convex hulls of barycentres of the
 *     vertices of the simplices, for some partition of the vertices.  For example
 *     case (a) is the special case of the tautological discrete partition. 
 *
 * We need a data type to specify the fixed point sets, with some redundancy to
 * make it readily-sortable and for debugging.  So perhaps a struct with:
 *  a) the dimension of the fixed point set
 *  b) the dimension of the simplex it lives in
 *  c) the particular simplex, and ideal end if its an ideal end
 *  d) the cycle decomposition that it corresponds to, i.e. a partition of the
 *     vertex set. 
 *
 * Then we need an algorithm to work out the boundary/incidence relations. We
 * will call the data type linearFacet. 
 */
std::vector< std::set< linearFacet >* > NSimplicialAutGrp::fixedPoints() const
{
 NCellularData* cDat;
 if (tri3) cDat = new NCellularData(*tri3);
      else cDat = new NCellularData(*tri4);
 std::vector< std::set< linearFacet >* > retval( fullMap.size() );
 // let's start with the homologyAction routine to determine which facets are 
 // preserved, and then identify the fixed-point set. 
 unsigned long domSim, ranSim; 

 for (unsigned long M=0; M<fullMap.size(); M++)
  {
   retval[M] = new std::set< linearFacet >;
   // TODO fill out retval[M] with fullMap[M] as base map. 
   for (unsigned long d=0; d<(tri3 ? 4 : 5); d++)
    {
     unsigned long stdCount( cDat->stdCellCount( d ) );
     // check if simplex of dimension d indexed by nicIx[d][i] is fixed.
     for (unsigned long domFac=0; domFac<stdCount; domFac++) if (tri3)
      {
       NPerm4 mPerm4, domPerm4;
       switch (d) {
       case 0: 
         domSim = cDat->nicIxLookup( tri3->getVertex(  cDat->nicIndex( d, domFac ) )->getEmbedding(0).getTetrahedron() );
         domPerm4 = tri3->getVertex( cDat->nicIndex( d, domFac ) )->getEmbedding(0).getVertices();
         ranSim = fullMap[M].germ[ domSim ].pen; // target simplex
         mPerm4 = NPerm4::S4[ fullMap[M].germ[ domSim ].perm ]; // permutation
         if ( domFac == cDat->nicIxLookup( tri3->getSimplex( ranSim )->getVertex( mPerm4[ domPerm4[0] ] ) ) )
            retval[M]->insert( linearFacet(d, d, domFac) ); 
         else if (M==0) { std::cout<<"NSimplicialAutGrp::fixedpts() Error, missed 0-cell "<<domFac<<std::endl; exit(1); }
        break;
       case 1: {
         domSim = cDat->nicIxLookup( tri3->getEdge(  cDat->nicIndex( d, domFac ) )->getEmbedding(0).getTetrahedron() );
         domPerm4 = tri3->getEdge( cDat->nicIndex( d, domFac ) )->getEmbedding(0).getVertices();
         ranSim = fullMap[M].germ[ domSim ].pen; // target simplex
         mPerm4 = NPerm4::S4[ fullMap[M].germ[ domSim ].perm ]; // permutation

         unsigned long eNum( NEdge::edgeNumber[mPerm4[domPerm4[0]]][mPerm4[domPerm4[1]]] );
         NPerm4 ranPerm4( tri3->getSimplex( ranSim )->getEdgeMapping( eNum ) );
         if ( domFac == cDat->nicIxLookup( tri3->getSimplex( ranSim )->getEdge(eNum) ) )
           {
            if (ranPerm4[0] == mPerm4[domPerm4[0]])
             {
               linearFacet F( 1, 1, domFac );
               std::set< unsigned long > FP; FP.insert(0); F.vCentres.insert( centroid( FP ) ); 
               FP.clear(); FP.insert(1); F.vCentres.insert( centroid( FP ) );
               retval[M]->insert( F );
             } // ID on facet
            else
             { 
               linearFacet F( 0, 1, domFac );
               std::set< unsigned long > FP; FP.insert(0); FP.insert(1); F.vCentres.insert( centroid( FP ) ); 
               retval[M]->insert( F );
             } // inversion on facet
           }  // test on identity map
          else if (M==0) { std::cout<<"NSimplicialAutGrp::fixedpts() ERROR, missed 1-cell "<<domFac<<std::endl; exit(1); }
       } break;
       case 2: {
         domSim = cDat->nicIxLookup( tri3->getTriangle( cDat->nicIndex( d, domFac ) )->getEmbedding(0).getTetrahedron() );
         domPerm4 = tri3->getTriangle( cDat->nicIndex( d, domFac ) )->getEmbedding(0).getVertices();
         ranSim = fullMap[M].germ[ domSim ].pen; // target simplex
         mPerm4 = NPerm4::S4[ fullMap[M].germ[domSim].perm ]; // permutation

         unsigned long tNum( mPerm4[domPerm4[3]] );
         NPerm4 ranPerm4( tri3->getSimplex( ranSim )->getTriangleMapping( tNum ) );
         NPerm3 relPerm( ranPerm4.preImageOf( mPerm4[domPerm4[0]] ), 
                         ranPerm4.preImageOf( mPerm4[domPerm4[1]] ), 
                         ranPerm4.preImageOf( mPerm4[domPerm4[2]] ) );

         if ( domFac == cDat->nicIxLookup( tri3->getSimplex( ranSim )->getTriangle(tNum) ) ) 
            {
             if ( (relPerm[0]==0) && (relPerm[1]==1) ) // ID
              { 
                linearFacet F( 2, 2, domFac );
                std::set< unsigned long > FP; FP.insert(0); F.vCentres.insert( centroid( FP ) ); 
                FP.clear(); FP.insert(1); F.vCentres.insert( centroid( FP ) );
                FP.clear(); FP.insert(2); F.vCentres.insert( centroid( FP ) );
                retval[M]->insert( F );
              } else
             if ( (relPerm[0]==0) || (relPerm[1]==1) || (relPerm[2]==2) ) // refl
              {
                unsigned long IDX( (relPerm[0]==0) ? 0 : ( (relPerm[1]==1) ? 1 : 2 ) );
                linearFacet F( 1, 2, domFac );
                std::set< unsigned long > FP; 

                FP.insert(IDX); F.vCentres.insert( centroid( FP ) ); 
                FP.clear(); FP.insert( (IDX + 1) % 3); FP.insert( (IDX + 2) % 3 ); F.vCentres.insert( centroid( FP ) );
                retval[M]->insert( F );
              } else // 3-cycle
             {
                linearFacet F( 0, 2, domFac );
                std::set< unsigned long > FP; 

                FP.insert(0); FP.insert(1); FP.insert(2); F.vCentres.insert( centroid( FP ) ); 
                retval[M]->insert( F );
             }
            } else if (M==0) { std::cout<<"NSimplicialAutGrp::fixedpts() ERROR, missed 2-cell "<<domFac<<std::endl; exit(1); }
       } break;
       case 3: {
         domSim = cDat->nicIxLookup( tri3->getSimplex(  cDat->nicIndex( d, domFac ) ) ); // domFac == domSim no?
         mPerm4 = NPerm4::S4[ fullMap[M].germ[ domSim ].perm ]; // permutation
         ranSim = fullMap[M].germ[ domSim ].pen; // target simplex
         if ( domFac == cDat->nicIxLookup( tri3->getSimplex( ranSim ) ) )
            { // cases: [4] ID, [0] 4-cycle, [0] two 2-cycles, [1] 1 fp and 3-cycle, [2] 2 fp and 2-cycle. 
              std::set< unsigned long > fixedPts;
              for (unsigned long k=0; k<4; k++) if (mPerm4[k]==k) fixedPts.insert(k);
              switch (fixedPts.size()) {
                case 0: { // either (01)(23) or (0123), two cases! 
                    if (mPerm4[mPerm4[0]]==0) // order 2, 1-dim'l fixed pts
                     {
                      linearFacet F( 1, 3, domFac );
                      std::set< unsigned long > FP, FP2; FP.insert(0); FP.insert( mPerm4[0] );
                      F.vCentres.insert( centroid(FP) ); 
                      for (unsigned long K=0; K<4; K++) if (FP.count(K)==0) FP2.insert(K);
                      F.vCentres.insert( centroid(FP2) );
                      retval[M]->insert(F);
                     }
                    else // order 4, 0-dim'l fixed points
                     {
                      linearFacet F( 0, 3, domFac );
                      std::set< unsigned long > FP; FP.insert(0); FP.insert(1);
                      FP.insert(2); FP.insert(3); F.vCentres.insert( centroid(FP) );
                      retval[M]->insert(F);
                     }
                 } break;
                case 1: { // 1 fixed pt and a 3-cycle
                    linearFacet F( 1, 3, domFac );
                    std::set< unsigned long > FP;
                    F.vCentres.insert( centroid(fixedPts) );
                    for (unsigned long K=0; K<4; K++) if ( fixedPts.count(K)==0 )
                     { FP.insert(K); }
                    F.vCentres.insert( centroid(FP) );
                    retval[M]->insert(F);
                 } break;
                case 2: { // two fixed pts and a 2-cycle
                    linearFacet F( 2, 3, domFac );
                    std::set< unsigned long > FP;
                    for (std::set< unsigned long >::iterator K=fixedPts.begin(); K!=fixedPts.end(); K++)
                     { FP.insert( *K ); F.vCentres.insert( centroid(FP) ); FP.clear(); }
                    for (unsigned long K=0; K<4; K++) if ( fixedPts.count(K)==0 )
                     { FP.insert(K); }
                    F.vCentres.insert( centroid(FP) );
                    retval[M]->insert(F);
                 } break;
                case 4: { // identity map, fixes whole tetrahedron. 
                    linearFacet F( 0, 3, domFac );
                    std::set< unsigned long > FP;
                    FP.insert(0); F.vCentres.insert( centroid(FP) ); FP.clear();
                    FP.insert(1); F.vCentres.insert( centroid(FP) ); FP.clear();
                    FP.insert(2); F.vCentres.insert( centroid(FP) ); FP.clear();
                    FP.insert(3); F.vCentres.insert( centroid(FP) ); FP.clear();
                    retval[M]->insert(F);
                 } break;
                // TODO put in test for other cases...
               }
            } 
       } break;
      } // end switch (d) statement

     } // end nicIx[d][domFac] loop on domFac, dimension 3
    else { // dim 4 
       NPerm5 mPerm5, domPerm5;
       switch (d) {
       case 0: 
         domSim = cDat->nicIxLookup( tri4->getVertex( 
           cDat->nicIndex( d, domFac ) )->getEmbedding(0).getPentachoron() );
         domPerm5 = tri4->getVertex( cDat->nicIndex( d, domFac ) )->getEmbedding(0).getVertices();
         ranSim = fullMap[M].germ[ domSim ].pen; // target simplex
         mPerm5 = NPerm5::S5[ fullMap[M].germ[ domSim ].perm ]; // permutation
         if ( domFac == cDat->nicIxLookup( tri4->getSimplex( ranSim )->getVertex( mPerm5[ domPerm5[0] ] ) ) )
            retval[M]->insert( linearFacet(d, d, domFac) ); 
         else if (M==0) 
{ // TODO there's some error in the code above. The identity map appears to permute the vertices!
std::cout<<"\nNSimplicialAutGrp::fixedpts() ERROR.\n";
std::cout<<"domFac == "<<domFac<<" nicIndex == "<<cDat->nicIndex( d, domFac )<<
           " domSim == "<<domSim<<" ranSim == "<<ranSim<<" target vtx lookup in nicIx == "<<
           cDat->nicIxLookup( tri4->getSimplex( ranSim )->getVertex( mPerm5[ domPerm5[0] ] ) )<<std::endl;
std::cout<<"tri4 vertexIndex == "<<tri4->vertexIndex( tri4->getSimplex( ranSim )->getVertex( mPerm5[ domPerm5[0] ] ) )
         <<"\n";
std::cout<<cDat->stdCellCount(0)<<" 0-cells total. "<<std::endl;
for (unsigned long Q=0; Q<cDat->stdCellCount(0); Q++) std::cout<<cDat->nicIndex( 0, Q )<<" ";
std::cout<<"\n mPerm5 == "<<mPerm5<<"\n";
std::cout<<std::endl;
exit(1);
// M is the map index. 
}
        break;
       case 1: {
         domSim = cDat->nicIxLookup( tri4->getEdge( cDat->nicIndex( d, domFac ) )->getEmbedding(0).getPentachoron() );
         domPerm5 = tri4->getEdge( cDat->nicIndex( d, domFac ) )->getEmbedding(0).getVertices();
         ranSim = fullMap[M].germ[ domSim ].pen; // target simplex
         mPerm5 = NPerm5::S5[ fullMap[M].germ[ domSim ].perm ]; // permutation

         unsigned long eNum( Dim4Edge::edgeNumber[mPerm5[domPerm5[0]]][mPerm5[domPerm5[1]]] );
         NPerm5 ranPerm5( tri4->getSimplex( ranSim )->getEdgeMapping( eNum ) );
         if ( domFac == cDat->nicIxLookup( tri4->getSimplex( ranSim )->getEdge(eNum) ) )
           {
            if (ranPerm5[0] == mPerm5[domPerm5[0]])
             {
               linearFacet F( 1, 1, domFac );
               std::set< unsigned long > FP; FP.insert(0); F.vCentres.insert( centroid( FP ) ); 
               FP.clear(); FP.insert(1); F.vCentres.insert( centroid( FP ) );
               retval[M]->insert( F );
             } // ID on facet
            else
             { 
               linearFacet F( 0, 1, domFac );
               std::set< unsigned long > FP; FP.insert(0); FP.insert(1); F.vCentres.insert( centroid( FP ) ); 
               retval[M]->insert( F );
             } // inversion on facet
           } 
         else if (M==0) { std::cout<<"\nNSimplicialAutGrp::fixedpts() Error, missed 1-cell "<<domFac<<std::endl; exit(1); }
       } break;
       case 2: {
         domSim = cDat->nicIxLookup( tri4->getTriangle( cDat->nicIndex( d, domFac ) )->getEmbedding(0).getPentachoron() );
         domPerm5 = tri4->getTriangle( cDat->nicIndex( d, domFac ) )->getEmbedding(0).getVertices();
         ranSim = fullMap[M].germ[ domSim ].pen; // target simplex
         mPerm5 = NPerm5::S5[ fullMap[M].germ[ domSim ].perm ]; // permutation

         unsigned long tNum( Dim4Triangle::triangleNumber[mPerm5[domPerm5[0]]]
                                    [mPerm5[domPerm5[1]]][mPerm5[domPerm5[2]]] );

         NPerm5 ranPerm5( tri4->getSimplex( ranSim )->getTriangleMapping( tNum ) );
         NPerm3 relPerm( ranPerm5.preImageOf( mPerm5[domPerm5[0]] ), 
                         ranPerm5.preImageOf( mPerm5[domPerm5[1]] ), 
                         ranPerm5.preImageOf( mPerm5[domPerm5[2]] ) );

         if ( domFac == cDat->nicIxLookup( tri4->getSimplex( ranSim )->getTriangle(tNum) ) ) 
            {
             if ( (relPerm[0]==0) && (relPerm[1]==1) ) // ID
              { 
                linearFacet F( 2, 2, domFac );
                std::set< unsigned long > FP; FP.insert(0); F.vCentres.insert( centroid( FP ) ); 
                FP.clear(); FP.insert(1); F.vCentres.insert( centroid( FP ) );
                FP.clear(); FP.insert(2); F.vCentres.insert( centroid( FP ) );
                retval[M]->insert( F );
              } else
             if ( (relPerm[0]==0) || (relPerm[1]==1) || (relPerm[2]==2) ) // refl
              {
                unsigned long IDX( (relPerm[0]==0) ? 0 : ( (relPerm[1]==1) ? 1 : 2 ) );
                linearFacet F( 1, 2, domFac );
                std::set< unsigned long > FP; 

                FP.insert(IDX); F.vCentres.insert( centroid( FP ) ); 
                FP.clear(); FP.insert( (IDX + 1) % 3); FP.insert( (IDX + 2) % 3 ); F.vCentres.insert( centroid( FP ) );
                retval[M]->insert( F );
              } else // 3-cycle
             {
                linearFacet F( 0, 2, domFac );
                std::set< unsigned long > FP; 

                FP.insert(0); FP.insert(1); FP.insert(2); F.vCentres.insert( centroid( FP ) ); 
                retval[M]->insert( F );
             }
            } 
         else if (M==0) { std::cout<<"NSimplicialAutGrp::fixedpts() Error, missed 2-cell "<<domFac<<std::endl; exit(1); }
       } break;
       case 3: {
         domSim = cDat->nicIxLookup( tri4->getTetrahedron(  cDat->nicIndex( d, domFac ) )->getEmbedding(0).getPentachoron() );
         domPerm5 = tri4->getTetrahedron( cDat->nicIndex( d, domFac ) )->getEmbedding(0).getVertices();
         ranSim = fullMap[M].germ[ domSim ].pen; // target simplex
         mPerm5 = NPerm5::S5[ fullMap[M].germ[ domSim ].perm ]; // permutation
         unsigned long tNum( mPerm5[domPerm5[4]] );
         NPerm5 ranPerm5( tri4->getSimplex( ranSim )->getTetrahedronMapping( tNum ) );
         NPerm4 relPerm( ranPerm5.preImageOf( mPerm5[domPerm5[0]] ), 
                         ranPerm5.preImageOf( mPerm5[domPerm5[1]] ), 
                         ranPerm5.preImageOf( mPerm5[domPerm5[2]] ),
                         ranPerm5.preImageOf( mPerm5[domPerm5[3]] ) );
         if ( domFac == cDat->nicIxLookup( tri4->getPentachoron( ranSim )->getTetrahedron( tNum ) ) )
          { 
           std::set< unsigned long > fixedPts;
           for (unsigned long k=0; k<4; k++) if (relPerm[k]==k) fixedPts.insert(k);
           switch (fixedPts.size()) {
             case 0: { // either (01)(23) or (0123), two cases! 
                 if (relPerm[relPerm[0]]==0) // order 2, 1-dim'l fixed pts
                  {
                   linearFacet F( 1, 3, domFac );
                   std::set< unsigned long > FP, FP2; FP.insert(0); FP.insert( relPerm[0] );
                   F.vCentres.insert( centroid(FP) ); 
                   for (unsigned long K=0; K<4; K++) if (FP.count(K)==0) FP2.insert(K);
                   F.vCentres.insert( centroid(FP2) );
                   retval[M]->insert(F);
                  }
                 else // order 4, 0-dim'l fixed points
                  {
                   linearFacet F( 0, 3, domFac );
                   std::set< unsigned long > FP; FP.insert(0); FP.insert(1);
                   FP.insert(2); FP.insert(3); F.vCentres.insert( centroid(FP) );
                   retval[M]->insert(F);
                  }
              } break;
             case 1: { // 1 fixed pt and a 3-cycle
                 linearFacet F( 1, 3, domFac );
                 std::set< unsigned long > FP;
                 F.vCentres.insert( centroid(fixedPts) );
                 for (unsigned long K=0; K<4; K++) if ( fixedPts.count(K)==0 )
                  { FP.insert(K); }
                 F.vCentres.insert( centroid(FP) );
                 retval[M]->insert(F);
              } break;
             case 2: { // two fixed pts and a 2-cycle
                 linearFacet F( 2, 3, domFac );
                 std::set< unsigned long > FP;
                 for (std::set< unsigned long >::iterator K=fixedPts.begin(); K!=fixedPts.end(); K++)
                  { FP.insert( *K ); F.vCentres.insert( centroid(FP) ); FP.clear(); }
                 for (unsigned long K=0; K<4; K++) if ( fixedPts.count(K)==0 )
                  { FP.insert(K); }
                 F.vCentres.insert( centroid(FP) );
                 retval[M]->insert(F);
              } break;
             case 4: { // identity map, fixes whole tetrahedron. 
                 linearFacet F( 3, 3, domFac );
                 std::set< unsigned long > FP;
                 FP.insert(0); F.vCentres.insert( centroid(FP) ); FP.clear();
                 FP.insert(1); F.vCentres.insert( centroid(FP) ); FP.clear();
                 FP.insert(2); F.vCentres.insert( centroid(FP) ); FP.clear();
                 FP.insert(3); F.vCentres.insert( centroid(FP) ); FP.clear();
                 retval[M]->insert(F);
              } break;
             default: { std::cout<<"NSimplicialAutGrp::fixedpts() Error, unknown case 3."<<std::endl; exit(1); } break;
            }
          }
         else if (M==0) { std::cout<<"NSimplicialAutGrp::fixedpts() Error, missed 3-cell "<<domFac<<std::endl; exit(1); }
       } break;
       case 4: { // TODO redo!
         domSim = cDat->nicIxLookup( tri4->getSimplex( cDat->nicIndex( d, domFac ) ) ); 
         ranSim = fullMap[M].germ[ domSim ].pen; // target simplex
         mPerm5 = NPerm5::S5[ fullMap[M].germ[ domSim ].perm ]; // permutation
         if ( domFac == cDat->nicIxLookup( tri4->getSimplex( ranSim ) ) )
            { // cases: [0] 5-cycle, [0] 3-cycle and 2-cycle, 
              //        [1] 4-cycle, [1] two 2-cycles, [2] 3-cycle, 
              //        [3] 2-cycle, [5] ID
              std::set< unsigned long > fixedPts;
              for (unsigned long k=0; k<5; k++) if (mPerm5[k]==k) fixedPts.insert(k);

              std::set< unsigned long > twoPts;
              for (unsigned long k=0; k<5; k++) if ( (mPerm5[k] != k) && 
                                (mPerm5[mPerm5[k]]==k) ) twoPts.insert(k);
              switch (fixedPts.size()) {
                case 0: { // either (01)(234) or (01234), two cases! 
                    if (twoPts.size()==2) // order 2, 1-dim'l fixed pts
                     {
                      linearFacet F( 1, 4, domFac );
                      F.vCentres.insert( centroid( twoPts ) );
                      std::set< unsigned long > FP; 
                      for (unsigned long K=0; K<5; K++) if (twoPts.count(K)==0) FP.insert(K);
                      F.vCentres.insert( centroid(FP) );
                      retval[M]->insert(F);
                     }
                    else // order 4, 0-dim'l fixed points
                     {
                      linearFacet F( 0, 4, domFac );
                      std::set< unsigned long > FP; 
                      for (unsigned long K=0; K<5; K++) FP.insert(K); 
                      F.vCentres.insert( centroid(FP) );
                      retval[M]->insert(F);
                     }
                 } break;
                case 1: { // 1 fixed pt. (0123) or (01)(23)
                  if (twoPts.size()==4) // two 2-cycles and a fixed point
                    { 
                     linearFacet F( 2, 4, domFac );
                     F.vCentres.insert( centroid( fixedPts ) );
                     std::set< unsigned long > FP, FP2;
                     FP.insert( *twoPts.begin() ); FP.insert( mPerm5[ *twoPts.begin() ] );
                     F.vCentres.insert( centroid( FP ) ); 
                     for (unsigned long k=0; k<5; k++) if ( (fixedPts.count(k)==0) && (FP.count(k)==0) )
                        FP2.insert(k);
                     F.vCentres.insert( centroid( FP2 ) );
                     retval[M]->insert(F);
                    }
                   else
                    { // fixed pt and 4-cycle
                     linearFacet F( 1, 4, domFac );
                     std::set< unsigned long > FP;
                     F.vCentres.insert( centroid(fixedPts) );
                     for (unsigned long K=0; K<5; K++) if ( fixedPts.count(K)==0 )
                      { FP.insert(K); }
                     F.vCentres.insert( centroid(FP) );
                     retval[M]->insert(F);
                    }
                 } break;
                case 2: { // two fixed pts and a 3-cycle
                    linearFacet F( 2, 4, domFac );
                    std::set< unsigned long > FP;
                    for (std::set< unsigned long >::iterator K=fixedPts.begin(); K!=fixedPts.end(); K++)
                     { FP.insert( *K ); F.vCentres.insert( centroid(FP) ); FP.clear(); }
                    for (unsigned long K=0; K<5; K++) if ( fixedPts.count(K)==0 )
                     { FP.insert(K); }
                    F.vCentres.insert( centroid(FP) );
                    retval[M]->insert(F);
                 } break;
                case 3: { // 3 fixed points and a 2-cycle
                    linearFacet F( 3, 4, domFac );
                    std::set< unsigned long > FP;
                    for (std::set< unsigned long >::iterator K=fixedPts.begin(); K!=fixedPts.end(); K++)
                     { FP.insert( *K ); F.vCentres.insert( centroid(FP) ); FP.clear(); }
                    F.vCentres.insert( centroid( twoPts) );
                    retval[M]->insert(F);
                 } break;
                case 5: { // identity map, fixes whole tetrahedron. 
                    linearFacet F( 4, 4, domFac );
                    std::set< unsigned long > FP;
                    for (unsigned long k=0; k<5; k++) { FP.insert(k); F.vCentres.insert( centroid( FP ) );
                        FP.clear(); }                    
                    retval[M]->insert(F);
                 } break;
                // TODO put in test for other cases...
               }
            } 
         else if (M==0) { std::cout<<"\nNSimplicialAutGrp::fixedpts() Error missed 4-cell."<<std::endl; exit(1); }
       } break;
      } // end switch (d) statement
     } // end dim 4
    } // end dimension d loop
  } // end retval[M] construction

 return retval;
}

void NSimplicialAutGrp::writeTextLong(std::ostream& out) const
{
 std::vector<NMatrixInt*> h1A( homologyH1action() );
 bool orFlag( (tri3 ? tri3->isOrientable() : tri4->isOrientable() ) );
 for (unsigned long i=0; i<fullMap.size(); i++)
  {
   if (i != 0) out<<"\n";
   out<<"Aut "<<i;
   if (orFlag) { std::string sig("+");
    if (tri3) { if ( (NPerm4::S4[fullMap[i].germ[0].perm].sign() * 
                    tri3->getSimplex(0)->orientation() * 
                    tri3->getSimplex(fullMap[i].germ[0].pen)->orientation() ) < 0 
                   ) sig = "-"; }
    else {
    if ( (NPerm5::S5[fullMap[i].germ[0].perm].sign() * 
                   tri4->getSimplex(0)->orientation() *
                   tri4->getSimplex(fullMap[i].germ[0].pen)->orientation() ) < 0 
                ) sig = "-"; }
   out<<sig; }
   // now let's compute the action on H1...
   if (h1A[i]->entry(0,0) == 1) out<<"+"; else out<<"-";
   delete (h1A[i]);

   out<<": ";
   for (unsigned long j=0; j<fullMap[i].germ.size(); j++)
    { if (j!=0) out<<", "; else out<<" ";
      if (tri3) out<<j<<" -> "<<fullMap[i].germ[j].pen<<" "<< NPerm4::S4[fullMap[i].germ[j].perm]; 
      else      out<<j<<" -> "<<fullMap[i].germ[j].pen<<" "<< NPerm5::S5[fullMap[i].germ[j].perm]; 
    }
  }
 NGroupPresentation G( groupPresentation() );
 G.intelligentSimplify();
 out<<"\nOrder: "<<groupOrder()<<" Presentation: "<<G.compact();
}

unsigned long NSimplicialAutGrp::groupOrder() const
{
 return fullMap.size();
}

void NSimplicialAutGrp::writeTextShort(std::ostream& out) const
{
 NGroupPresentation G( groupPresentation() );
 G.intelligentSimplify();
 out<<"Order: "<<groupOrder()<<" Presentation: "<<G.compact();
}


// presentation of the full group of simplicial automorphisms.
NGroupPresentation NSimplicialAutGrp::groupPresentation() const
{
 NGroupPresentation retval;
 retval.addGenerator( fullMap.size() );
 for (unsigned long i=0; i<fullMap.size(); i++)
  for (unsigned long j=0; j<fullMap.size(); j++)
   {
    // multiply fullMap[i] * fullMap[j] = prod...
    NGroupExpression* relij( new NGroupExpression );
    relij->addTermLast( i, 1 );
    relij->addTermLast( j, 1 );
    // determine product.  First determine how tets/pens are permuted. 
    isoStruct composite;
    composite.germ.resize( fullMap[0].germ.size() );
    for (unsigned long k=0; k<fullMap[0].germ.size(); k++)
     { // fullMap[j] sends pen k to fullMap[j].germ[k].pen via perm perm
       composite.germ[k].pen = fullMap[i].germ[ fullMap[j].germ[k].pen ].pen;
       composite.germ[k].perm = (tri3 ? 
        (NPerm4::S4[fullMap[i].germ[ fullMap[j].germ[k].pen ].perm] *
        NPerm4::S4[fullMap[j].germ[k].perm]).S4Index() :
        (NPerm5::S5[fullMap[i].germ[ fullMap[j].germ[k].pen ].perm] *
        NPerm5::S5[fullMap[j].germ[k].perm]).S5Index() );
     }
    // fullMap has been made binary searchable so we now just look up
    //  composite in fullMap. No we'll use lower_bound
    std::vector< isoStruct >::const_iterator CI = lower_bound( fullMap.begin(), fullMap.end(), composite );
    if ( CI == fullMap.end() ) 
      { std::cout<<"NSimplicialGroup::groupPresentation() ERROR product not found. "<<std::endl; exit(1); }
    relij->addTermLast( CI - fullMap.begin(), -1 ); 
    retval.addRelation( relij ); 
   }
 
 return retval;
}

bool isoStruct::operator<(const isoStruct &rhs) const
{
 if (germ.size() < rhs.germ.size()) return true; 
 if (germ.size() > rhs.germ.size()) return false; 
 for (unsigned long i=0; i<germ.size(); i++)
  {
    if (germ[i].pen < rhs.germ[i].pen) return true; 
    if (germ[i].pen > rhs.germ[i].pen) return false; 
    if (germ[i].perm < rhs.germ[i].perm) return true;
    if (germ[i].perm > rhs.germ[i].perm) return false;
  }
 return false;
}    



// TODO: fix return type and get it to do something useful.
void NCellularData::simplicialAutomorphismGroup() const
 {
  
 }




} // namespace regina
