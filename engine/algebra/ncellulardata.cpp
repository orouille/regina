
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Computational Engine                                                  *
 *                                                                        *
 *  Copyright (c) 1999-2009, Ben Burton                                   *
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

#include "maths/matrixops.h"
#include "maths/nprimes.h"
#include "algebra/ncellulardata.h"

#include <map>
#include <list>
#include <cmath>

#include <iostream>
#include <sstream>

namespace regina {


/*void dumpMatrix( const NMatrixRing<NLargeInteger> &mat )
{
for (unsigned long j=0; j<mat.rows(); j++)
 {
 std::cout<<"[";
 for (unsigned long i=0; i<mat.columns(); i++)
  std::cout<<mat.entry(j,i)<<" ";
 std::cout<<"]\n";
 }
}*/

/*
template <class T>
void dumpVector( const std::vector<T> &vec )
{
for (unsigned long j=0; j<vec.size(); j++)
 {  if (j != 0) std::cout<<" ";
    std::cout<<vec[j]; }
}
*/

void correctRelOrMat( NMatrixInt &CM, unsigned long domdim, const NTriangulation* tri3, const Dim4Triangulation* tri4, 
 const std::vector< std::vector<unsigned long> > &dcIx ); // forward reference, see ncellulardata_init.cpp for details

const NAbelianGroup* NCellularData::unmarkedGroup( const GroupLocator &g_desc) const
{
std::map< GroupLocator, NAbelianGroup* >::const_iterator p;
p = abelianGroups.find(g_desc);
if (p != abelianGroups.end()) return (p->second);
// okay, so now we know there's no group matching g_desc in markedAbelianGroups, so we make one.
NAbelianGroup* gptr;

std::vector< NMatrixInt* > CC; // choose the right chain complex
if (g_desc.hcs == DUAL_coord) CC = dCC; else if (g_desc.hcs == STD_coord) CC = sCC; else
if (g_desc.hcs == MIX_coord) CC = mCC; else if (g_desc.hcs == STD_BDRY_coord) CC = sbCC; else
if (g_desc.hcs == STD_REL_BDRY_coord) CC = srCC;

if (g_desc.var == coVariant) // homology requested
 {
  if (g_desc.cof == 0) gptr = new NAbelianGroup( *CC[g_desc.dim], *CC[g_desc.dim+1] );
   else gptr = new NAbelianGroup( *CC[g_desc.dim], *CC[g_desc.dim+1], NLargeInteger(g_desc.cof) );

  std::map< GroupLocator, NAbelianGroup* > *abgptr = 
	const_cast< std::map< GroupLocator, NAbelianGroup* > *> (&abelianGroups);
  abgptr->insert(std::pair<GroupLocator,NAbelianGroup*>( g_desc, gptr ) ); 
  return gptr;
 }
else // cohomology requested
 { 
  NMatrixInt dCCN( CC[g_desc.dim]->columns(),   CC[g_desc.dim]->rows() );
  for (unsigned long i=0; i<dCCN.rows(); i++) for (unsigned long j=0; j<dCCN.columns(); j++)
   dCCN.entry(i,j) = CC[g_desc.dim]->entry(j,i);
  NMatrixInt dCCM( CC[g_desc.dim+1]->columns(), CC[g_desc.dim+1]->rows() );
  for (unsigned long i=0; i<dCCM.rows(); i++) for (unsigned long j=0; j<dCCM.columns(); j++)
   dCCM.entry(i,j) = CC[g_desc.dim+1]->entry(j,i);
  if (g_desc.cof == 0) gptr = new NAbelianGroup( dCCM, dCCN );
   else gptr = new NAbelianGroup( dCCM, dCCN, NLargeInteger(g_desc.cof) );
  std::map< GroupLocator, NAbelianGroup* > *abgptr = 
	const_cast< std::map< GroupLocator, NAbelianGroup* > *> (&abelianGroups);
  abgptr->insert(std::pair<GroupLocator,NAbelianGroup*>(g_desc,gptr)); 
  return gptr;
 }

return NULL;
}

// todo add an aDim and ensure request is with dimension bounds
const NMarkedAbelianGroup* NCellularData::markedGroup( const GroupLocator &g_desc) const
{
std::map< GroupLocator, NMarkedAbelianGroup* >::const_iterator p;
p = markedAbelianGroups.find(g_desc);
if (p != markedAbelianGroups.end()) return (p->second);
// okay, so now we know there's no group matching g_desc in markedAbelianGroups, so we make one.
NMarkedAbelianGroup* mgptr;

std::vector< NMatrixInt* > CC; // choose the right chain complex
if (g_desc.hcs == DUAL_coord) CC = dCC; else if (g_desc.hcs == STD_coord) CC = sCC; else
if (g_desc.hcs == MIX_coord) CC = mCC; else if (g_desc.hcs == STD_BDRY_coord) CC = sbCC; else
if (g_desc.hcs == STD_REL_BDRY_coord) CC = srCC;

if (g_desc.var == coVariant) // homology requested
 {
  if (g_desc.cof == 0) mgptr = new NMarkedAbelianGroup( *CC[g_desc.dim], *CC[g_desc.dim+1] );
   else mgptr = new NMarkedAbelianGroup( *CC[g_desc.dim], *CC[g_desc.dim+1], NLargeInteger(g_desc.cof) );

  std::map< GroupLocator, NMarkedAbelianGroup* > *mabgptr = 
	const_cast< std::map< GroupLocator, NMarkedAbelianGroup* > *> (&markedAbelianGroups);
  mabgptr->insert(std::pair<GroupLocator,NMarkedAbelianGroup*>(g_desc,mgptr)); 
  return mgptr;
 }
else // cohomology requested
 { 
  NMatrixInt dCCN( CC[g_desc.dim]->columns(),   CC[g_desc.dim]->rows() );
  for (unsigned long i=0; i<dCCN.rows(); i++) for (unsigned long j=0; j<dCCN.columns(); j++)
   dCCN.entry(i,j) = CC[g_desc.dim]->entry(j,i);
  NMatrixInt dCCM( CC[g_desc.dim+1]->columns(), CC[g_desc.dim+1]->rows() );
  for (unsigned long i=0; i<dCCM.rows(); i++) for (unsigned long j=0; j<dCCM.columns(); j++)
   dCCM.entry(i,j) = CC[g_desc.dim+1]->entry(j,i);
  if (g_desc.cof == 0) mgptr = new NMarkedAbelianGroup( dCCM, dCCN );
   else mgptr = new NMarkedAbelianGroup( dCCM, dCCN, NLargeInteger(g_desc.cof) );
  std::map< GroupLocator, NMarkedAbelianGroup* > *mabgptr = 
	const_cast< std::map< GroupLocator, NMarkedAbelianGroup* > *> (&markedAbelianGroups);
  mabgptr->insert(std::pair<GroupLocator,NMarkedAbelianGroup*>(g_desc,mgptr)); 
  return mgptr;
 }

return NULL;
}

// TODO: coefficient LES maps like Bockstein
const NHomMarkedAbelianGroup* NCellularData::homGroup( const HomLocator &h_desc) const
{
 std::map< HomLocator, NHomMarkedAbelianGroup* >::const_iterator p;
 p = homMarkedAbelianGroups.find(h_desc);
 if (p != homMarkedAbelianGroups.end()) return (p->second);
 // okay, so now we know there's no group matching g_desc in markedAbelianGroups, so we make one.
 unsigned long aDim = ( tri3 ? 3 : 4 ); // ambient dimension
 // out-of-bounds request
 if ( (h_desc.domain.dim > aDim) || (h_desc.range.dim > aDim) ) return NULL;
 // inappropriate change of coefficients request
 if ( h_desc.range.cof != 0 ) if ( h_desc.domain.cof % h_desc.range.cof != 0 ) return NULL;
 // choose the right chain map, or leave unallocated if we can't make sense of the request.
 NMatrixInt* CM(NULL); 

 if (h_desc.domain.var == h_desc.range.var) // variance-preserving map requested
  { // check if they want a pure change-of-coefficients map 
   if ( (h_desc.domain.dim == h_desc.range.dim) && (h_desc.domain.hcs == h_desc.range.hcs) )
    {
      unsigned long ccdim = markedGroup( h_desc.domain )->getRankCC();
      CM = new NMatrixInt(ccdim, ccdim); CM->makeIdentity(); 
    }

   // check if they want a subdivision-induced map, co-variant
   if ( ( (h_desc.domain.hcs == STD_coord) || (h_desc.domain.hcs == DUAL_coord) ) && 
          (h_desc.domain.var == coVariant) && (h_desc.range.hcs == MIX_coord) )
         CM = ( h_desc.domain.hcs == STD_coord ? clonePtr(smCM[h_desc.domain.dim]) :
					         clonePtr(dmCM[h_desc.domain.dim]) );
   if ( ( (h_desc.range.hcs == STD_coord) || (h_desc.range.hcs == DUAL_coord) ) && // contravariant 
          (h_desc.domain.var == contraVariant)  && (h_desc.domain.hcs == MIX_coord) )
      {
	 const NMatrixInt* tCMp( h_desc.range.hcs == STD_coord ? smCM[h_desc.domain.dim] : 
			                                         dmCM[h_desc.domain.dim] );
         CM = new NMatrixInt( tCMp->columns(), tCMp->rows() );
         for (unsigned long i=0; i<CM->rows(); i++) for (unsigned long j=0; j<CM->columns(); j++)
	  CM->entry( i, j ) = tCMp->entry( j, i );
      }

   // check if they want a map from the homology LES of the pair (M, \partial M) 
   if ( h_desc.domain.var == coVariant )
    { // \partial M --> M
      if ( (h_desc.domain.hcs == STD_BDRY_coord) && (h_desc.range.hcs == STD_coord) &&
           (h_desc.domain.dim == h_desc.range.dim) && (h_desc.domain.dim < aDim) )
	CM = clonePtr( sbiCM[h_desc.domain.dim] ); 
      // M --> (M, \partial M)
      else if ( (h_desc.domain.hcs == STD_coord) && (h_desc.range.hcs == STD_REL_BDRY_coord) &&
           (h_desc.domain.dim == h_desc.range.dim) )
	CM = clonePtr( strCM[h_desc.domain.dim] );
      // (M, \partial M) --> \partial M
      else if ( (h_desc.domain.hcs == STD_REL_BDRY_coord) && (h_desc.range.hcs == STD_BDRY_coord) &&
           (h_desc.domain.dim == h_desc.range.dim+1) && (h_desc.range.dim < aDim) )
	CM = clonePtr( schCM[h_desc.range.dim] );
   }
   else
    { // \partial M <-- M
      if ( (h_desc.domain.hcs == STD_coord) && (h_desc.range.hcs == STD_BDRY_coord) &&
           (h_desc.domain.dim == h_desc.range.dim) && (h_desc.range.dim < aDim) )
       {	 
         CM = new NMatrixInt( sbiCM[h_desc.domain.dim]->columns(), sbiCM[h_desc.domain.dim]->rows() );
         for (unsigned long i=0; i<CM->rows(); i++) for (unsigned long j=0; j<CM->columns(); j++)
	  CM->entry( i, j ) = sbiCM[h_desc.domain.dim]->entry( j, i );
       }
      // M <-- (M, \partial M)
      else if ( (h_desc.domain.hcs == STD_REL_BDRY_coord) && (h_desc.range.hcs == STD_coord) &&
           (h_desc.domain.dim == h_desc.range.dim) )
       {	 
         CM = new NMatrixInt( strCM[h_desc.domain.dim]->columns(), strCM[h_desc.domain.dim]->rows() );
         for (unsigned long i=0; i<CM->rows(); i++) for (unsigned long j=0; j<CM->columns(); j++)
	  CM->entry( i, j ) = strCM[h_desc.domain.dim]->entry( j, i );
       }
      // (M, \partial M) <-- \partial M
      else if ( (h_desc.domain.hcs == STD_BDRY_coord) && (h_desc.range.hcs == STD_REL_BDRY_coord) &&
           (h_desc.domain.dim+1 == h_desc.range.dim) && (h_desc.domain.dim < aDim) )
       {	 
         CM = new NMatrixInt( schCM[h_desc.domain.dim]->columns(), schCM[h_desc.domain.dim]->rows() );
         for (unsigned long i=0; i<CM->rows(); i++) for (unsigned long j=0; j<CM->columns(); j++)
	  CM->entry( i, j ) = schCM[h_desc.domain.dim]->entry( j, i );
       }
    } 
 }
 else
 { // variance-reversing map requested
   // record if manifold orientable or not
   bool orientable = false;
   if ( tri3 ) if ( tri3->isOrientable() ) orientable = true;
   if ( tri4 ) if ( tri4->isOrientable() ) orientable = true;
   if ( (!orientable) && (h_desc.domain.cof != 2) && (h_desc.range.cof != 2) ) return NULL;
 
   // check if they want Poincare Duality
   if ( (h_desc.domain.var == coVariant) && (h_desc.domain.hcs == DUAL_coord) &&
        (h_desc.range.hcs == STD_REL_BDRY_coord) && (h_desc.domain.dim + h_desc.range.dim == aDim) )
   { 
     CM = new NMatrixInt( numRelativeCells[ h_desc.range.dim ], numDualCells[ h_desc.domain.dim ] );
     CM->makeIdentity(); // good enough if not orientable, but we need to correct if orientable. 
     if (orientable) correctRelOrMat( *CM, h_desc.domain.dim, tri3, tri4, dcIx );
   } else 
   if ( (h_desc.domain.var == contraVariant) && (h_desc.domain.hcs == DUAL_coord) &&
        (h_desc.range.hcs == STD_REL_BDRY_coord) && (h_desc.domain.dim + h_desc.range.dim == aDim) )
   { 
     CM = new NMatrixInt( numRelativeCells[ h_desc.range.dim ], numDualCells[ h_desc.domain.dim ] );
     CM->makeIdentity(); // good enough if not orientable, but we need to correct if orientable. 
     if (orientable) correctRelOrMat( *CM, h_desc.domain.dim, tri3, tri4, dcIx );   
   }
 }

 NHomMarkedAbelianGroup* hmgptr(NULL);
 if ( CM ) // we found the requested map, now make sure we have the domain and range, then we're happy.
 {
   //  ensure we have domain and range
   const NMarkedAbelianGroup* dom = markedGroup( h_desc.domain );
   const NMarkedAbelianGroup* ran = markedGroup( h_desc.range );
   if ( dom && ran )
    {
     hmgptr = new NHomMarkedAbelianGroup( *dom, *ran, *CM );
     std::map< HomLocator, NHomMarkedAbelianGroup* > *hmabgptr = 
      const_cast< std::map< HomLocator, NHomMarkedAbelianGroup* > *> (&homMarkedAbelianGroups);
     hmabgptr->insert(std::pair<HomLocator,NHomMarkedAbelianGroup*>(h_desc,hmgptr)); 
     delete CM;
     return hmgptr;
    }
 }
 if ( CM ) delete CM;

// didn't find what was requested
return NULL;
}

NSVPolynomialRing NCellularData::poincarePolynomial() const
{
NSVPolynomialRing retval;
unsigned long aDim( tri3 ? 3 : 4 );
for (unsigned long i=0; i<=aDim; i++) retval += 
 NSVPolynomialRing( NLargeInteger( unmarkedGroup( GroupLocator(i, coVariant, DUAL_coord, 0))->getRank() ), i );
return retval;
}


/**
 *  Computes various bilinear forms associated to the homology of the manifold:
 *
 *  1) Homology-Cohomology pairing <.,.>  ie: H_i(M;R) x H^i(M;R) --> R  where R is the coefficients
 *  2) Intersection product               ie: H_i(M;R) x H_j(M;R) --> H_{(i+j)-n}(M;R)
 *  3) Torsion linking form               ie: H_i(M;Z) x H_j(M;Z) --> H_{(i+j)-(n-1)}(M;Q/Z)
 *  4) cup products                       ie: H^i(M;R) x H^j(M;R) --> H^{i+j}(M;R)
 *
 *  What has been implemented:
 *
 *  ALL of (1) and (2). 
 *
 *  3) 3-dimensional torsion linking form
 *
 *  4) nothing
 *
 *  TODO:
 *
 *  (2) for new coordinate systems MIX_BDRY_coord, MIX_REL_BDRY_coord, DUAL_BDRY_coord, DUAL_REL_BDRY_coord
 *
 *  3) aDim == 4:  (2,1)->0, (1,2)->0
 *
 *  4) all -- implement via Poincare duality once (2) complete
 */
const NBilinearForm* NCellularData::bilinearForm( const FormLocator &f_desc ) const
{
 unsigned long aDim( tri3 ? 3 : 4 );
 NBilinearForm* bfptr(NULL);

 std::map< FormLocator, NBilinearForm* >::const_iterator p;
 p = bilinearForms.find(f_desc);
 if (p != bilinearForms.end()) return (p->second);
 // okay, so now we know there's no form matching f_desc in bilinearForms, so we make one.

 // case 1: homology-cohomology pairing
 if ( ( f_desc.ft == evaluationForm ) &&
      ( f_desc.ldomain.dim == f_desc.rdomain.dim ) && (f_desc.ldomain.var != f_desc.rdomain.var) &&
      ( f_desc.ldomain.cof == f_desc.rdomain.cof ) && (f_desc.ldomain.hcs == f_desc.rdomain.hcs) ) 
  { // homology-cohomology pairing definable.
   const NMarkedAbelianGroup* lDom( markedGroup(f_desc.ldomain) );
   const NMarkedAbelianGroup* rDom( markedGroup(f_desc.rdomain) );
   NMarkedAbelianGroup rAng( 1, f_desc.rdomain.cof );

   NSparseGridRing< NLargeInteger > intM(3); 
   NMultiIndex x(3); 

   for (unsigned long i=0; i<lDom->getRankCC(); i++)
    { x[0] = i; x[1] = i; x[2] = 0;
      intM.setEntry( x, 1 ); } 

   bfptr = new NBilinearForm( *lDom, *rDom, rAng, intM );
   std::map< FormLocator, NBilinearForm* > *mbfptr = 
      const_cast< std::map< FormLocator, NBilinearForm* > *> (&bilinearForms);
   mbfptr->insert( std::pair<FormLocator, NBilinearForm*>(f_desc, bfptr) );
   return bfptr; 
  }

 // case 2: intersection products i+j >= n == aDim
 if ( ( f_desc.ft == intersectionForm ) &&
      ( f_desc.ldomain.var == coVariant ) && (f_desc.rdomain.var == coVariant) &&
      ( f_desc.ldomain.dim + f_desc.rdomain.dim >= aDim ) &&
      ( (f_desc.ldomain.dim + f_desc.rdomain.dim) - aDim < aDim - 1 ) &&
      ( f_desc.ldomain.dim > 0) && ( f_desc.rdomain.dim > 0 ) &&
      ( f_desc.ldomain.cof == f_desc.rdomain.cof ) &&
      ( f_desc.ldomain.hcs == DUAL_coord ) && (f_desc.rdomain.hcs == STD_REL_BDRY_coord) )
  {// check its orientable if R != Z_2
   if ( (f_desc.ldomain.cof != 2) && ( tri3 ? !tri3->isOrientable() : !tri4->isOrientable() ) ) return NULL;
   const NMarkedAbelianGroup* lDom( markedGroup(f_desc.ldomain) );
   const NMarkedAbelianGroup* rDom( markedGroup(f_desc.rdomain) );
   const NMarkedAbelianGroup* rAng( markedGroup( GroupLocator( (f_desc.ldomain.dim + f_desc.rdomain.dim) - aDim,
					coVariant, MIX_coord, f_desc.ldomain.cof ) ) );
   NSparseGridRing< NLargeInteger > intM(3); 
   
   if (aDim == 3) // aDim==3  
    {

     if ( (f_desc.ldomain.dim == 2) && (f_desc.rdomain.dim == 2) ) // (dual)H_2 x (std_rel)H_2 --> (mix)H_1
       for (unsigned long i=0; i<numRelativeCells[2]; i++)
        { // each STD_REL_BDRY cell has <= 3 boundary 1-cells, each one corresponds to a DUAL cell...
         const NFace* fac( tri3->getFace( rIx[2][i] ) ); const NEdge* edg(NULL);
         const NTetrahedron* tet( fac->getEmbedding(1).getTetrahedron() );
         for (unsigned long j=0; j<3; j++)
	  {
	   edg = fac->getEdge(j); if (!edg->isBoundary())
	    { // intM[ J, i, 2*numNonIdealCells[2] + 3*i+j ] += whatever
	      // for orientation we need to compare normal orientation of these edges to product normal orientations
             unsigned long J( lower_bound( dcIx[2].begin(), dcIx[2].end(), tri3->edgeIndex( edg ) ) - dcIx[2].begin() );
	     NMultiIndex x(3); x[0] = J; x[1] = i; x[2] = 2*numNonIdealCells[1] + 3*rIx[2][i] + j;
	     // fac->getEdgeMapping(j)[0] and [1] are the vertices of the edge in the face, so we apply
	     // facinc to that, then get the corresp edge number
	     NPerm4 facinc( fac->getEmbedding(1).getVertices() );
	     NPerm4 edginc( tet->getEdgeMapping( NEdge::edgeNumber[facinc[(j+1)%3]][facinc[(j+2)%3]] ) );
             // edginc[2,3] give orientation of part of dual 2-cell in this tet...
	     // normalize edginc to ambient orientation
	     if (tet->orientation() != edginc.sign()) edginc = edginc * NPerm4(0,1);
	     int inoutor = ((tet->orientation() == facinc.sign()) ? 1 : -1);
	     NPerm4 dualor( facinc[j], edginc[0], edginc[1], facinc[3]);           
             intM.setEntry( x, dualor.sign()*inoutor*tet->orientation() );
	    }
  	  }
        }

     if ( (f_desc.ldomain.dim == 2) && (f_desc.rdomain.dim == 1) )// (dual)H_2 x (std_rel)H_1 --> (mix)H_0
       for (unsigned long i=0; i<numRelativeCells[1]; i++)
        {
         const NEdge* edg( tri3->getEdge( rIx[1][i] ) ); 
         const NTetrahedron* tet( edg->getEmbedding(0).getTetrahedron() );
         unsigned long J( lower_bound( dcIx[2].begin(), dcIx[2].end(), rIx[1][i] ) - dcIx[2].begin() );
         NMultiIndex x(3); x[0] = J; x[1] = i; x[2] = numNonIdealCells[0] + i;
         NPerm4 edginc( edg->getEmbedding(0).getVertices() );
         intM.setEntry( x, edginc.sign()*tet->orientation() ); 
        }

     if ( (f_desc.ldomain.dim == 1) && (f_desc.rdomain.dim == 2) )// (dual)H_1 x (std_rel)H_2 --> (mix)H_0 
       for (unsigned long i=0; i<numRelativeCells[2]; i++)
        {
         const NFace* fac( tri3->getFace( rIx[2][i] ) ); 
         const NTetrahedron* tet( fac->getEmbedding(0).getTetrahedron() );
         unsigned long J( lower_bound( dcIx[1].begin(), dcIx[1].end(), rIx[2][i] ) - dcIx[1].begin() );
         NMultiIndex x(3); x[0] = J; x[1] = i; x[2] = numNonIdealCells[0] + numNonIdealCells[1] + i;
         NPerm4 facinc( fac->getEmbedding(0).getVertices() );
         intM.setEntry( x, facinc.sign()*tet->orientation() ); 
        }

     bfptr = new NBilinearForm( *lDom, *rDom, *rAng, intM );
     std::map< FormLocator, NBilinearForm* > *mbfptr = 
      const_cast< std::map< FormLocator, NBilinearForm* > *> (&bilinearForms);
     mbfptr->insert( std::pair<FormLocator, NBilinearForm*>(f_desc, bfptr) );
     return bfptr; 
    } // end aDim == 3

   
   if ( aDim == 4 ) // aDim==4 
    {
     if ( (f_desc.ldomain.dim == 2) && (f_desc.rdomain.dim == 2) ) // H_2 x H_2 --> H_0
       for (unsigned long i=0; i<numRelativeCells[2]; i++)
        {
         const Dim4Face* fac( tri4->getFace( rIx[2][i] ) );
         const Dim4Pentachoron* pen( fac->getEmbedding(0).getPentachoron() );
         NPerm5 facinc( fac->getEmbedding(0).getVertices() );
         unsigned long J( lower_bound( dcIx[2].begin(), dcIx[2].end(), rIx[2][i] ) - dcIx[2].begin() );
         NMultiIndex x(3); x[0] = J; x[1] = i; x[2] = numNonIdealCells[0] + numNonIdealCells[1] + i;
         intM.setEntry( x, facinc.sign() * pen->orientation() );
        }
     if ( (f_desc.ldomain.dim == 1) && (f_desc.rdomain.dim == 3) )// (dual)H_1 x (std_rel_bdry)H_3 --> H_0
       for (unsigned long i=0; i<numRelativeCells[3]; i++)
        {
         const Dim4Tetrahedron* tet( tri4->getTetrahedron( rIx[3][i] ) );
         const Dim4Pentachoron* pen( tet->getEmbedding(0).getPentachoron() );
         NPerm5 tetinc( tet->getEmbedding(0).getVertices() );
         unsigned long J( lower_bound( dcIx[1].begin(), dcIx[1].end(), rIx[3][i] ) - dcIx[1].begin() );
         NMultiIndex x(3); x[0] = J; x[1] = i; x[2] = numNonIdealCells[0] + numNonIdealCells[1] + numNonIdealCells[2] + i;
         intM.setEntry( x, tetinc.sign() * pen->orientation() );
        }
     if ( (f_desc.ldomain.dim == 3) && (f_desc.rdomain.dim == 1) )// (dual)H_3 x (std_rel_bdry)H_1 --> H_0
      for (unsigned long i=0; i<numRelativeCells[1]; i++)
        {
         const Dim4Edge* edg( tri4->getEdge( rIx[1][i] ) );
         const Dim4Pentachoron* pen( edg->getEmbedding(0).getPentachoron() );
         NPerm5 edginc( edg->getEmbedding(0).getVertices() );
         unsigned long J( lower_bound( dcIx[3].begin(), dcIx[3].end(), rIx[1][i] ) - dcIx[3].begin() );
         NMultiIndex x(3); x[0] = J; x[1] = i; x[2] = numNonIdealCells[0] + numNonIdealCells[1] + numNonIdealCells[2] + i;
         intM.setEntry( x, edginc.sign() * pen->orientation() );
        }

     if ( (f_desc.ldomain.dim == 2) && (f_desc.rdomain.dim == 3) )// (dual)H_2 x (std_rel_bdry)H_3 --> H_1
       for (unsigned long i=0; i<numRelativeCells[3]; i++)
        { // each STD_REL_BDRY cell has <= 3 boundary 1-cells, each one corresponds to a DUAL cell...
         const Dim4Tetrahedron* tet( tri4->getTetrahedron( rIx[3][i] ) ); const Dim4Face* fac(NULL);
         const Dim4Pentachoron* pen( tet->getEmbedding(1).getPentachoron() );
         NPerm5 tetinc( tet->getEmbedding(1).getVertices() );
         for (unsigned long j=0; j<4; j++)
	  {
	   fac = tet->getFace(j); if (!fac->isBoundary())
	    { // intM[ J, i, 2*numNonIdealCells[1] + 3*numNonIdealCells[2] + 4*i+j ] += whatever
	      // for orientation we need to compare normal orientation of intersection to product normal orientations
             unsigned long J( lower_bound( dcIx[2].begin(), dcIx[2].end(), tri4->faceIndex( fac ) ) - dcIx[2].begin() );
	     NMultiIndex x(3); x[0] = J; x[1] = i; x[2] = 2*numNonIdealCells[1] + 3*numNonIdealCells[2] + 4*rIx[3][i] + j;
	     NPerm5 facinc( pen->getFaceMapping( Dim4Face::faceNumber[tetinc[(j+1)%4]][tetinc[(j+2)%4]][tetinc[(j+3)%4]] ) ); 
             // adjust for coherent oriented normal fibres
	     if (facinc.sign() != pen->orientation()) facinc=facinc*NPerm5(0,1);
	     int inoutor = ( (tetinc.sign() == pen->orientation()) ? 1 : -1 );	     
             // the intersection is the edge from the centre of tet to the centre of fac
             // so the intersection edge + normal orientation is represented by tetinc[j], facinc[0,1,2], tetnum in pen
	     NPerm5 dualor( tetinc[j], facinc[0], facinc[1], facinc[2],  tet->getEmbedding(1).getTetrahedron());           
             intM.setEntry( x, dualor.sign()*pen->orientation()*inoutor ); 
	    }
  	  }
        }
	
     if ( (f_desc.ldomain.dim == 3) && (f_desc.rdomain.dim == 2) )// (dual)H_3 x (std_red_bdry)H_2 --> H_1
       for (unsigned long i=0; i<numRelativeCells[2]; i++)
        { // each STD_REL_BDRY cell has <= 3 boundary 1-cells, each one corresponds to a DUAL cell...
         const Dim4Face* fac( tri4->getFace( rIx[2][i] ) ); const Dim4Edge* edg(NULL);
         const Dim4Pentachoron* pen( fac->getEmbedding(0).getPentachoron() );
         NPerm5 facinc( fac->getEmbedding(0).getVertices() );
         for (unsigned long j=0; j<3; j++)
	  {
	   edg = fac->getEdge(j); if (!edg->isBoundary())
	    { // intM[ J, i, 2*numNonIdealCells[1] + 3*numNonIdealCells[2] + 4*i+j ] += whatever
	      // for orientation we need to compare normal orientation of intersection to product normal orientations
             unsigned long J( lower_bound( dcIx[3].begin(), dcIx[3].end(), tri4->edgeIndex( edg ) ) - dcIx[3].begin() );
	     NMultiIndex x(3); x[0] = J; x[1] = i; x[2] = 2*numNonIdealCells[1] + 3*rIx[2][i] + j; 
	     NPerm5 edginc( pen->getEdgeMapping( Dim4Edge::edgeNumber[facinc[(j+1)%3]][facinc[(j+2)%3]] ) ); 
	     // adjust for coherent oriented normal fibres
	     if (facinc.sign() != pen->orientation()) facinc = facinc*NPerm5(3,4);
	     if (edginc.sign() != pen->orientation()) edginc = edginc*NPerm5(0,1);
	     // the intersection is the edge from the centre of fac to the centre of edg.
	     // so the intersection edge + normal orientation is represented by facinc[j], edginc[0,1], facinc[3,4]

	     NPerm5 dualor( facinc[j], edginc[0], edginc[1], facinc[3],  facinc[4]); 
             intM.setEntry( x, dualor.sign()*pen->orientation() );
	    }
  	  }
        }

     if ( (f_desc.ldomain.dim == 3) && (f_desc.rdomain.dim == 3) )// (dual)H_3 x (std_rel_bdry)H_3 --> H_2
       for (unsigned long i=0; i<numRelativeCells[3]; i++)
        { // each STD_REL_BDRY cell has <= 3 boundary 1-cells, each one corresponds to a DUAL cell...
         const Dim4Tetrahedron* tet( tri4->getTetrahedron( rIx[3][i] ) ); const Dim4Edge* edg(NULL);
         const Dim4Pentachoron* pen( tet->getEmbedding(1).getPentachoron() );
         NPerm5 tetinc( tet->getEmbedding(1).getVertices() ); // [0,1,2,3]->tet in pen, 4->tet num in pen.
         for (unsigned long j=0; j<6; j++)
	  {
	   edg = tet->getEdge(j); if (!edg->isBoundary())
	    { // intM[ J, i, 2*numNonIdealCells[1] + 3*numNonIdealCells[2] + 4*i+j ] += whatever
	      // for orientation we need to compare normal orientation of intersection to product normal orientations
             unsigned long J( lower_bound( dcIx[3].begin(), dcIx[3].end(), tri4->edgeIndex( edg ) ) - dcIx[3].begin() );
	     NMultiIndex x(3); x[0] = J; x[1] = i; x[2] = 3*numNonIdealCells[2] + 6*i + j;
	     NPerm5 edgintet( tet->getEdgeMapping( j ) ); // [0,1] --> verts in tet, 4->4. 
             NPerm5 ordual2cell( tetinc * edgintet ); // [0,1] --> verts in pen, 4->tet in pen
	     NPerm5 edginc( pen->getEdgeMapping( Dim4Edge::edgeNumber[ordual2cell[0]][ordual2cell[1]] ) );
	     // adjust for coherent oriented normal fibres
	     if (edginc.sign() != pen->orientation()) edginc = edginc*NPerm5(0,1);
	     int inoutor = ( (tetinc.sign() == pen->orientation()) ? 1 : -1 );	     
             // intersection is the mixed 2-cell corresp to dual 2-cell in tet, dual to edge j.
	     // combined orientation NPerm5 consists of tetinc[ NEdge::ordering[j][2], NEdge::ordering[j][3] ], 
	     //  edginc[0], edginc[1], tet->getEmbedding(1).getTetrahedron()

	     NPerm5 dualor( ordual2cell[2], ordual2cell[3], edginc[0], edginc[1], 
			    tet->getEmbedding(1).getTetrahedron() ); 
             intM.setEntry( x, dualor.sign()*pen->orientation()*inoutor );
	    }
  	  }
        }

     bfptr = new NBilinearForm( *lDom, *rDom, *rAng, intM );
     std::map< FormLocator, NBilinearForm* > *mbfptr = 
      const_cast< std::map< FormLocator, NBilinearForm* > *> (&bilinearForms);
     mbfptr->insert( std::pair<FormLocator, NBilinearForm*>(f_desc, bfptr) );
     return bfptr; 
    } // end aDim == 4
  } // end DUAL x STD_REL_BDRY intersectionform loop

 // convienience intersection product pairings
 if ( ( f_desc.ft == intersectionForm ) && ( f_desc.ldomain.var == coVariant ) && (f_desc.rdomain.var == coVariant) &&
      ( f_desc.ldomain.dim + f_desc.rdomain.dim >= aDim ) && ( (f_desc.ldomain.dim + f_desc.rdomain.dim) - aDim < aDim - 1 ) &&
      ( f_desc.ldomain.dim > 0) && ( f_desc.rdomain.dim > 0 ) && ( f_desc.ldomain.cof == f_desc.rdomain.cof ) &&
      ( f_desc.ldomain.hcs == DUAL_coord ) && (f_desc.rdomain.hcs == DUAL_coord) )
  { // convienience pairing -- the DUAL x DUAL --> MIX pairing
      GroupLocator dc( f_desc.rdomain.dim, coVariant, DUAL_coord,         f_desc.rdomain.cof );
      GroupLocator mc( f_desc.rdomain.dim, coVariant, MIX_coord,          f_desc.rdomain.cof );
      GroupLocator sc( f_desc.rdomain.dim, coVariant, STD_coord,          f_desc.rdomain.cof );
      GroupLocator sb( f_desc.rdomain.dim, coVariant, STD_REL_BDRY_coord, f_desc.rdomain.cof );
      const NHomMarkedAbelianGroup* sc_sb(homGroup( HomLocator( sc, sb ) ) );
      const NHomMarkedAbelianGroup* sc_mc(homGroup( HomLocator( sc, mc ) ) );
      const NHomMarkedAbelianGroup* dc_mc(homGroup( HomLocator( dc, mc ) ) );
      NHomMarkedAbelianGroup f( (*sc_sb) * (sc_mc->inverseHom()) * (*dc_mc) );
      FormLocator prim(f_desc); prim.rdomain.hcs = STD_REL_BDRY_coord;
      bfptr = new NBilinearForm( bilinearForm(prim)->rCompose(f) );
      std::map< FormLocator, NBilinearForm* > *mbfptr = 
       const_cast< std::map< FormLocator, NBilinearForm* > *> (&bilinearForms);
      mbfptr->insert( std::pair<FormLocator, NBilinearForm*>(f_desc, bfptr) );
      return bfptr; 
  }
 if ( ( f_desc.ft == intersectionForm ) && ( f_desc.ldomain.var == coVariant ) && (f_desc.rdomain.var == coVariant) &&
      ( f_desc.ldomain.dim + f_desc.rdomain.dim >= aDim ) && ( (f_desc.ldomain.dim + f_desc.rdomain.dim) - aDim < aDim - 1 ) &&
      ( f_desc.ldomain.dim > 0) && ( f_desc.rdomain.dim > 0 ) && ( f_desc.ldomain.cof == f_desc.rdomain.cof ) &&
      ( f_desc.ldomain.hcs == STD_coord ) && (f_desc.rdomain.hcs == STD_REL_BDRY_coord) )
  { // convienience pairing -- the STD x STD_REL_BDRY --> MIX pairing
      GroupLocator dc( f_desc.rdomain.dim, coVariant, DUAL_coord,         f_desc.rdomain.cof );
      GroupLocator mc( f_desc.rdomain.dim, coVariant, MIX_coord,          f_desc.rdomain.cof );
      GroupLocator sc( f_desc.rdomain.dim, coVariant, STD_coord,          f_desc.rdomain.cof );
      const NHomMarkedAbelianGroup* sc_mc(homGroup( HomLocator( sc, mc ) ) );
      const NHomMarkedAbelianGroup* dc_mc(homGroup( HomLocator( dc, mc ) ) );
      NHomMarkedAbelianGroup f( (dc_mc->inverseHom()) * (*sc_mc) );
      FormLocator prim(f_desc); prim.ldomain.hcs = DUAL_coord;
      bfptr = new NBilinearForm( bilinearForm(prim)->lCompose(f) );
      std::map< FormLocator, NBilinearForm* > *mbfptr = 
       const_cast< std::map< FormLocator, NBilinearForm* > *> (&bilinearForms);
      mbfptr->insert( std::pair<FormLocator, NBilinearForm*>(f_desc, bfptr) );
      return bfptr; 
  }

 // case 3: torsion linking forms
 if ( ( f_desc.ft == torsionlinkingForm) && ( f_desc.ldomain.var == coVariant ) && (f_desc.rdomain.var == coVariant) &&
      ( f_desc.ldomain.dim + f_desc.rdomain.dim + 1 == aDim ) && ( f_desc.ldomain.dim > 0) && ( f_desc.rdomain.dim > 0 ) &&
      ( f_desc.ldomain.cof == 0 ) && (f_desc.rdomain.cof == 0 ) && 
      ( f_desc.ldomain.hcs == DUAL_coord ) && (f_desc.rdomain.hcs == STD_REL_BDRY_coord) )
  { // step 1: construct range, ldomain and rdomain.  We'll make range Z_n where n=the gcd of the order of the torsion subgroups of
    //         ldomain and rdomain respectively... ie: n == gcd(a,b) a, b largest inv. facs ldomain,rdomain.
    GroupLocator ldd( f_desc.ldomain.dim, f_desc.ldomain.var, f_desc.ldomain.hcs, f_desc.ldomain.cof );
    GroupLocator rdd( f_desc.rdomain.dim, f_desc.rdomain.var, f_desc.rdomain.hcs, f_desc.rdomain.cof );
    const NMarkedAbelianGroup* ld(markedGroup(ldd));     const NMarkedAbelianGroup* rd(markedGroup(rdd));
     // now we build ldomain and rdomain
    NMatrixInt presL( ld->getNumberOfInvariantFactors(), ld->getNumberOfInvariantFactors() );
    NMatrixInt presR( rd->getNumberOfInvariantFactors(), rd->getNumberOfInvariantFactors() );
    NMatrixInt lnull( 1, ld->getNumberOfInvariantFactors() );
    NMatrixInt rnull( 1, rd->getNumberOfInvariantFactors() );
    for (unsigned long i=0; i<ld->getNumberOfInvariantFactors(); i++)
	presL.entry(i,i) = ld->getInvariantFactor(i); 
    for (unsigned long i=0; i<rd->getNumberOfInvariantFactors(); i++)
	presR.entry(i,i) = rd->getInvariantFactor(i); 
    NMarkedAbelianGroup ldomain( lnull, presL );
    NMarkedAbelianGroup rdomain( rnull, presR );
    NLargeInteger N(NLargeInteger::one);
    if ( !ldomain.isTrivial() && !rdomain.isTrivial() ) N=ld->getInvariantFactor( ld->getNumberOfInvariantFactors()-1 ).gcd(
							rd->getInvariantFactor( rd->getNumberOfInvariantFactors()-1 ) );
    NMarkedAbelianGroup range( 1, N ); // Z_N with triv pres 0 --> Z --N--> Z --> Z_N --> 0
    NSparseGridRing< NLargeInteger > intM(3); 

    // step 2: dimension-specific constructions
    if (aDim == 3)
     {
	for (unsigned long i=0; i<ld->getNumberOfInvariantFactors(); i++)
         {
 	 for (unsigned long j=0; j<rd->getNumberOfInvariantFactors(); j++)
	  {
	   // take ccRep(j), multiply by order rd->getInvariantFactor(j), apply writeAsBoundary, 
           std::vector< NLargeInteger > rFac( rd->getTorsionRep(j) );
           for (unsigned long k=0; k<rFac.size(); k++) rFac[k]*=rd->getInvariantFactor(j);
           std::vector< NLargeInteger > std_rel_bdry_2vec( rd->writeAsBoundary( rFac ) );
           std::vector< NLargeInteger > dual_1vec( ld->getTorsionRep(i) );
	   // intersect with ld->getInvariantFactor(i)
	   NLargeInteger sum(NLargeInteger::zero);
           for (unsigned long k=0; k<dual_1vec.size(); k++)
            {
             const NFace* fac( tri3->getFace( rIx[2][i] ) ); 
             const NTetrahedron* tet( fac->getEmbedding(0).getTetrahedron() );
             NPerm4 facinc( fac->getEmbedding(0).getVertices() );
            sum += std_rel_bdry_2vec[k]*dual_1vec[k]*facinc.sign()*tet->orientation(); // orientation convention...
            }
           // rescale sum, check if relevant, append to intM if so...
           sum *= (N / rd->getInvariantFactor(j));
           sum %= N; if (sum < NLargeInteger::zero) sum += N;
           NMultiIndex x(3); x[0] = i; x[1] = j; x[2] = 0; 
           if (sum != NLargeInteger::zero) intM.setEntry( x, sum );
	  }
         }
     }
    
    if ( (aDim == 4) && (f_desc.ldomain.dim == 2) )
     {
	for (unsigned long i=0; i<ld->getNumberOfInvariantFactors(); i++)
 	 for (unsigned long j=0; j<rd->getNumberOfInvariantFactors(); j++)
	  {
	   // take ccRep(j), multiply by order rd->getInvariantFactor(j), apply writeAsBoundary, 
           std::vector< NLargeInteger > rFac( rd->getTorsionRep(j) );
           for (unsigned long k=0; k<rFac.size(); k++) rFac[k]*=rd->getInvariantFactor(j);
           std::vector< NLargeInteger > std_rel_bdry_2vec( rd->writeAsBoundary( rFac ) );
           std::vector< NLargeInteger > dual_1vec( ld->getTorsionRep(i) );
	   // intersect with ld->getInvariantFactor(i)
	   NLargeInteger sum(NLargeInteger::zero);
           for (unsigned long k=0; k<dual_1vec.size(); k++)
            {
             const Dim4Face* fac( tri4->getFace( rIx[2][i] ) ); 
             const Dim4Pentachoron* pen( fac->getEmbedding(0).getPentachoron() );
             NPerm5 facinc( fac->getEmbedding(0).getVertices() );
             sum += std_rel_bdry_2vec[k]*dual_1vec[k]*facinc.sign()*pen->orientation(); // orientation convention...
            }
           // rescale sum, check if relevant, append to intM if so...
           sum *= (N / rd->getInvariantFactor(j));
           sum %= N; if (sum < NLargeInteger::zero) sum += N;
           NMultiIndex x(3); x[0] = i; x[1] = j; x[2] = 0; 
           if (sum != NLargeInteger::zero) intM.setEntry( x, sum );
	  }
     } 
    if ( (aDim == 4) && (f_desc.ldomain.dim == 1) )
     {
 	for (unsigned long i=0; i<ld->getNumberOfInvariantFactors(); i++)
 	 for (unsigned long j=0; j<rd->getNumberOfInvariantFactors(); j++)
	  {
	   // take ccRep(j), multiply by order rd->getInvariantFactor(j), apply writeAsBoundary, 
           std::vector< NLargeInteger > rFac( rd->getTorsionRep(j) );
           for (unsigned long k=0; k<rFac.size(); k++) rFac[k]*=rd->getInvariantFactor(j);
           std::vector< NLargeInteger > std_rel_bdry_2vec( rd->writeAsBoundary( rFac ) );
           std::vector< NLargeInteger > dual_1vec( ld->getTorsionRep(i) );
	   // intersect with ld->getInvariantFactor(i)
	   NLargeInteger sum(NLargeInteger::zero);
           for (unsigned long k=0; k<dual_1vec.size(); k++)
            {
             const Dim4Tetrahedron* tet( tri4->getTetrahedron( rIx[1][i] ) ); 
             const Dim4Pentachoron* pen( tet->getEmbedding(1).getPentachoron() );
             NPerm5 tetinc( tet->getEmbedding(1).getVertices() );
             sum += std_rel_bdry_2vec[k]*dual_1vec[k]*tetinc.sign()*pen->orientation(); // orientation convention...
            }
           // rescale sum, check if relevant, append to intM if so...
           sum *= (N / rd->getInvariantFactor(j));
           sum %= N; if (sum < NLargeInteger::zero) sum += N;
           NMultiIndex x(3); x[0] = i; x[1] = j; x[2] = 0; 
           if (sum != NLargeInteger::zero) intM.setEntry( x, sum );	  }
    } 

     bfptr = new NBilinearForm( ldomain, rdomain, range, intM );
     std::map< FormLocator, NBilinearForm* > *fptr = 
      const_cast< std::map< FormLocator, NBilinearForm* > *> (&bilinearForms);
     fptr->insert( std::pair<FormLocator, NBilinearForm*>(f_desc, bfptr) );
     return bfptr; 
  }

 // convienience torsion linking pairings
 if ( ( f_desc.ft == torsionlinkingForm ) && ( f_desc.ldomain.var == coVariant ) && (f_desc.rdomain.var == coVariant) &&
      ( f_desc.ldomain.dim + f_desc.rdomain.dim + 1 == aDim ) && ( f_desc.ldomain.dim > 0) && ( f_desc.rdomain.dim > 0 ) &&
      ( f_desc.ldomain.cof == 0 ) && ( f_desc.rdomain.cof == 0 ) &&
      ( f_desc.ldomain.hcs == DUAL_coord ) && (f_desc.rdomain.hcs == DUAL_coord) )
  { // convienience pairing -- the DUAL x DUAL pairing
    // the natural pairing is in DUAL x STD_REL_BDRY coords, so we provide the change of coordinates...
      GroupLocator dc( f_desc.rdomain.dim, coVariant, DUAL_coord,         f_desc.rdomain.cof );
      GroupLocator mc( f_desc.rdomain.dim, coVariant, MIX_coord,          f_desc.rdomain.cof );
      GroupLocator sc( f_desc.rdomain.dim, coVariant, STD_coord,          f_desc.rdomain.cof );
      GroupLocator sb( f_desc.rdomain.dim, coVariant, STD_REL_BDRY_coord, f_desc.rdomain.cof );
      const NHomMarkedAbelianGroup* sc_sb(homGroup( HomLocator( sc, sb ) ) );
      const NHomMarkedAbelianGroup* sc_mc(homGroup( HomLocator( sc, mc ) ) );
      const NHomMarkedAbelianGroup* dc_mc(homGroup( HomLocator( dc, mc ) ) );

      NMatrixInt rnull( 1, dc_mc->getDomain().getNumberOfInvariantFactors() );
      NMatrixInt rpres( dc_mc->getDomain().getNumberOfInvariantFactors(), dc_mc->getDomain().getNumberOfInvariantFactors() );
      for (unsigned long i=0; i<dc_mc->getDomain().getNumberOfInvariantFactors(); i++)
 	rpres.entry(i,i) = dc_mc->getDomain().getInvariantFactor(i);
      NMarkedAbelianGroup rtrivG( rnull, rpres );
      NMatrixInt rMap( dc_mc->getDomain().getRankCC(), dc_mc->getDomain().getNumberOfInvariantFactors() );
      for (unsigned long j=0; j<rMap.columns(); j++)
 	{
         std::vector<NLargeInteger> jtor( dc_mc->getDomain().getTorsionRep(j) );
         for (unsigned long i=0; i<rMap.rows(); i++) 
	  rMap.entry( i, j ) = jtor[i];
	}
      NHomMarkedAbelianGroup rinc( rtrivG, dc_mc->getDomain(), rMap );
     
      NMatrixInt lnull( 1, sc_sb->getRange().getNumberOfInvariantFactors() );
      NMatrixInt lpres( sc_sb->getRange().getNumberOfInvariantFactors(), sc_sb->getRange().getNumberOfInvariantFactors() );
      for (unsigned long i=0; i<sc_sb->getRange().getNumberOfInvariantFactors(); i++)
	lpres.entry(i,i) = sc_sb->getRange().getInvariantFactor(i);
      NMarkedAbelianGroup ltrivG( lnull, lpres );
      NMatrixInt lMap( sc_sb->getRange().getNumberOfInvariantFactors(), sc_sb->getRange().getRankCC() );    
      for (unsigned long j=0; j<lMap.columns(); j++)
	{
	 std::vector<NLargeInteger> jtor( sc_sb->getRange().snfRep( sc_sb->getRange().cycleProjection( j ) ) );
         // entries  0 .. < sc_sb->getRange().getNum... relevant. 
         for (unsigned long i=0; i<lMap.rows(); i++)
	  lMap.entry( i, j ) = jtor[i];
	}
      NHomMarkedAbelianGroup lproj( sc_sb->getRange(), ltrivG, lMap );

      NHomMarkedAbelianGroup f( lproj * (*sc_sb) * (sc_mc->inverseHom()) * (*dc_mc) * rinc ); // dual -> std_rel_bdry
      FormLocator prim(f_desc); prim.rdomain.hcs = STD_REL_BDRY_coord;

      bfptr = new NBilinearForm( bilinearForm(prim)->rCompose(f) ); 
      std::map< FormLocator, NBilinearForm* > *mbfptr =             
       const_cast< std::map< FormLocator, NBilinearForm* > *> (&bilinearForms);
      mbfptr->insert( std::pair<FormLocator, NBilinearForm*>(f_desc, bfptr) );
      return bfptr; 
  }
 if ( ( f_desc.ft == torsionlinkingForm ) && ( f_desc.ldomain.var == coVariant ) && (f_desc.rdomain.var == coVariant) &&
      ( f_desc.ldomain.dim + f_desc.rdomain.dim + 1 == aDim ) && ( f_desc.ldomain.dim > 0) && ( f_desc.rdomain.dim > 0 ) &&
      ( f_desc.ldomain.cof == 0 ) && ( f_desc.rdomain.cof == 0 ) &&
      ( f_desc.ldomain.hcs == STD_coord ) && (f_desc.rdomain.hcs == STD_coord) )
  { // convienience pairing -- the STD x STD pairing
    // natural pairing is DUAL x STD_REL_BDRY coords, so we need DUAL -> STD and STD -> STD_REL_BDRY maps
      GroupLocator dc( f_desc.rdomain.dim, coVariant, DUAL_coord,         f_desc.rdomain.cof );
      GroupLocator mc( f_desc.rdomain.dim, coVariant, MIX_coord,          f_desc.rdomain.cof );
      GroupLocator sc( f_desc.rdomain.dim, coVariant, STD_coord,          f_desc.rdomain.cof );
      GroupLocator sb( f_desc.rdomain.dim, coVariant, STD_REL_BDRY_coord, f_desc.rdomain.cof );
      const NHomMarkedAbelianGroup* sc_sb(homGroup( HomLocator( sc, sb ) ) ); // STD --> STD_REL_BDRY
      const NHomMarkedAbelianGroup* sc_mc(homGroup( HomLocator( sc, mc ) ) );
      const NHomMarkedAbelianGroup* dc_mc(homGroup( HomLocator( dc, mc ) ) );
      NHomMarkedAbelianGroup fl( (sc_mc->inverseHom())*(*dc_mc) ); // DUAL -> STD
      FormLocator prim(f_desc); prim.ldomain.hcs = DUAL_coord; prim.rdomain.hcs = STD_REL_BDRY_coord;
      bfptr = new NBilinearForm( bilinearForm(prim)->lCompose(fl).rCompose(*sc_sb) ); 
      std::map< FormLocator, NBilinearForm* > *mbfptr = 
       const_cast< std::map< FormLocator, NBilinearForm* > *> (&bilinearForms);
      mbfptr->insert( std::pair<FormLocator, NBilinearForm*>(f_desc, bfptr) );
      return bfptr; 
  }

 // case 4: cup products
 //         a) std_rel_bdry x dual
 //         b) std_rel_bdry x std_rel_bdry
 //         c) std_rel_bdry x std
 //         d) std x std
 //         e) dual x dual
 if ( ( f_desc.ft == cupproductForm ) &&
      ( f_desc.ldomain.var == contraVariant ) && ( f_desc.rdomain.var == contraVariant ) &&
      ( f_desc.ldomain.dim + f_desc.rdomain.dim <= aDim ) &&
      ( f_desc.ldomain.dim > 0 ) && ( f_desc.rdomain.dim > 0 )&&
      ( f_desc.ldomain.cof == f_desc.rdomain.cof ) &&
      ( f_desc.ldomain.hcs == DUAL_coord ) && (f_desc.rdomain.hcs == STD_REL_BDRY_coord) )
  {// TODO 
  }
 // dual pairing DUAL x STD_REL_BDRY
 // dual cp1     ( f_desc.ldomain.hcs == DUAL_coord ) && (f_desc.rdomain.hcs == DUAL_coord) )
 // dual cp2    ( f_desc.ldomain.hcs == STD_coord ) && (f_desc.rdomain.hcs == STD_REL_BDRY_coord) )
 // PD maps (dual)H_k --> (std_rel_bdry)H^{n-k} and
 //         (dual)H^k --> (std_rel_bdry)H_{n-k}
 // oh, to get H^i M x H^j M --> H^{i+j} M we need dual_boundary and dual_rel_bdry coords, and
 // all the relevant maps, and dual_rel_bdry --> std poincare duality map... a bunch more work. 
 return NULL;
}

const NGroupPresentation* NCellularData::groupPresentation( const GroupPresLocator &g_desc ) const
{
 reloop_loop:

 // various bail triggers
 if ( (g_desc.sub_man==ideal_boundary) &&    (g_desc.component_index >= numIdealBdryComps) ) return NULL;
 if ( (g_desc.sub_man==standard_boundary) && (g_desc.component_index >= numStdBdryComps) )   return NULL;

 std::map< GroupPresLocator, NGroupPresentation* >::const_iterator p;
 p = groupPresentations.find(g_desc);
 if (p != groupPresentations.end()) return (p->second);
 else 
  { 
   buildFundGrpPres(); // ensure it's computed, and look it up.
   goto reloop_loop;
  }
 // return NULL if an invalid request
 return NULL;
}

const NHomGroupPresentation* NCellularData::homGroupPresentation( const HomGroupPresLocator &h_desc ) const
{
 reloop_loop:
 std::map< HomGroupPresLocator, NHomGroupPresentation* >::const_iterator p;

 // various bail triggers
 if ( (h_desc.inclusion_sub_man==ideal_boundary) &&    (h_desc.subman_component_index >= numIdealBdryComps) ) return NULL;
 if ( (h_desc.inclusion_sub_man==standard_boundary) && (h_desc.subman_component_index >= numStdBdryComps) )   return NULL;

 p = homGroupPresentations.find(h_desc);
 if (p != homGroupPresentations.end()) return (p->second);
 else 
  { 
   buildFundGrpPres(); // ensure it's computed, and look it up.
   goto reloop_loop;
  }
 // return NULL if an invalid request
 return NULL;
}

unsigned long NCellularData::components( submanifold_type ctype ) const
{
 if (ctype == whole_manifold) return 1;
 if (ctype == standard_boundary) return stdBdryPi1Gen.size();
 if (ctype == ideal_boundary) return idBdryPi1Gen.size();
} 

unsigned long NCellularData::cellCount(homology_coordinate_system hcs, unsigned dimension) const
{
if ( (dimension > 4) && tri4 ) return 0; 
if ( (dimension > 3) && tri3 ) return 0; // out of bounds check
if (hcs == STD_coord) return numStandardCells[dimension]; 
if (hcs == DUAL_coord) return numDualCells[dimension]; 
if (hcs == MIX_coord) return numMixCells[dimension];
if (hcs == MIX_REL_BDRY_coord) return numMixRelCells[dimension]; 
if (hcs == STD_REL_BDRY_coord) return numRelativeCells[dimension]; 
if (hcs == DUAL_REL_BDRY_coord) return numDualRelCells[dimension]; 
if ( (dimension > 3) && tri4 ) return 0;
if ( (dimension > 2) && tri3 ) return 0;
if (hcs == STD_BDRY_coord) return numStandardBdryCells[dimension]; 
if (hcs == MIX_BDRY_coord) return numMixBdryCells[dimension]; 
if (hcs == DUAL_BDRY_coord) return numDualBdryCells[dimension]; 
return 0; // only get here if hcs out of bounds!
}

long int NCellularData::eulerChar() const
{ return numDualCells[0]-numDualCells[1]+numDualCells[2]-numDualCells[3]+numDualCells[4]; }

long int NCellularData::signature() const
{
 if (tri3) return 0; if (!tri4->isOrientable()) return 0;
 const NBilinearForm* b = bilinearForm( 
       FormLocator( intersectionForm, GroupLocator(2, coVariant, DUAL_coord, 0), GroupLocator(2, coVariant, DUAL_coord, 0) ) );
 return b->signature();
}


const NMatrixInt* NCellularData::integerChainComplex( const ChainComplexLocator &c_desc ) const
{
 reloop_loop:
 std::map< ChainComplexLocator, NMatrixInt* >::const_iterator p;
 // various bail triggers

 p = integerChainComplexes.find(c_desc);
 if (p != integerChainComplexes.end()) return (p->second);
 else 
  { 
   // do something! TODO
   // std::vector< NMatrixInt* > sCC, sbCC, srCC, dCC, dbCC, drCC, mCC, mbCC, mrCC;
   // what we'll do is rewrite all the routines in ncellulardata.init.cc.cpp to ones which
   // produce vectors of vectors of structs { incident to which cells, sign, lift in to \tilde M, ie elt of pi1(M) }
   //  so data[i][j] indicates add the data describing how the j-th facet of the i-th cell in the chain complex. 
   //                      oh.  We should store this as an NSparseGridRing! 
   // struct facetData { unsigned long cellNo; signed long sig; NGroupExpression translate; }
   goto reloop_loop;
  }
 // return NULL if an invalid request
 return NULL;
}

const NMatrixInt* NCellularData::integerChainMap( const ChainMapLocator &m_desc ) const
{
 reloop_loop:
 std::map< ChainMapLocator, NMatrixInt* >::const_iterator p;
 // various bail triggers
 p = integerChainMaps.find(m_desc);
 if (p != integerChainMaps.end()) return (p->second);
 else 
  { 
   // do something! TODO
   // std::vector< NMatrixInt* > sbiCM, strCM, schCM,   dbiCM, dtrCM, dchCM,   mbiCM, mtrCM, mchCM,  
   //			       smCM, dmCM,            smbCM, dmbCM,          srmCM, drmCM;

   goto reloop_loop;
  }
 // return NULL if an invalid request
 return NULL;
}


} // namespace regina



