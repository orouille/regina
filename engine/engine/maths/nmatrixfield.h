
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

/*! \file nmatrixfield.h
 *  \brief Deals with matrices of elements from a given field.
 */

#ifndef __NMATRIXFIELD_H
#ifndef __DOXYGEN
#define __NMATRIXFIELD_H
#endif

#include "maths/nmatrix.h"

namespace regina {

/**
 * \weakgroup maths
 * @{
 */

/**
 * Represents a matrix of elements from a given field T.
 *
 * \pre Type T has a default constructor and overloads the assignment
 * (<tt>=</tt>) operator.
 * \pre An element <i>t</i> of type T can be written to an output stream
 * <i>out</i> using the standard expression <tt>out << t</tt>.
 * \pre Type T provides binary operators <tt>+</tt>, <tt>-</tt>, <tt>*</tt> and
 * <tt>/</tt> and unary operators <tt>+=</tt>, <tt>*=</tt> and <tt>/=</tt>.
 *
 * \ifacespython Not present.
 */
template <class T>
class NMatrixField : public NMatrixRing<T> {
    public:
        /**
         * Creates a new matrix of the given size.
         * All entries will be initialised using their default
         * constructors.
         *
         * \pre The given number of rows and columns are
         * both strictly positive.
         *
         * @param rows the number of rows in the new matrix.
         * @param cols the number of columns in the new matrix.
         */
        NMatrixField(unsigned long rows, unsigned long cols) :
                NMatrixRing<T>(rows, cols) {
        }
        /**
         * Creates a new matrix that is a clone of the given matrix.
         *
         * @param cloneMe the matrix to clone.
         */
        NMatrixField(const NMatrix<T>& cloneMe) :
                NMatrixRing<T>(cloneMe) {
        }

        /**
         * Divides the given row by the given factor.
         *
         * Note that \a factor is passed by value in case it is an
         * element of the row to be changed.
         *
         * \pre The given row is between 0 and rows()-1 inclusive.
         *
         * @param row the row to work with.
         * @param factor the factor by which to divide the given row.
         */
        void divRow(unsigned long row, T factor) {
            for (unsigned long i = 0; i < this->nCols; i++)
                this->data[row][i] /= factor;
        }
        /**
         * Divides the given column by the given factor.
         *
         * Note that \a factor is passed by value in case it is an
         * element of the row to be changed.
         *
         * \pre The given column is between 0 and columns()-1 inclusive.
         *
         * @param column the column to work with.
         * @param factor the factor by which to divide the given column.
         */
        void divCol(unsigned long column, T factor) {
            for (unsigned long i = 0; i < this->nRows; i++)
                this->data[i][column] /= factor;
        }

        /**
         * Diagonalises this matrix by applying only row operations.
         * This matrix will be changed directly.  Details of how the
         * diagonalisation was done will be returned.
         *
         * Note that this matrix need not be invertible and need not
         * even be square.
         *
         * @return a newly created matrix M for which
         * <tt>M * old = D</tt>, where \c old is the initial value of
         * this matrix and \c D is the diagonalised matrix.
         * M will be a square matrix of size rows().
         */
        NMatrixField<T>* diagonaliseRow() {
            NMatrixField<T>* ans = new NMatrixField(this->nRows, this->nRows);
            ans->makeIdentity();

            unsigned long doneRow = 0;
            unsigned long doneCol = 0;

            unsigned long row;
            while (doneCol < this->nCols && doneRow < this->nRows) {
                // Is there a non-zero value in this column?
                row = doneRow;
                while (row < this->nRows &&
                        this->data[row][doneCol] == NMatrixRing<T>::zero)
                    row++;
                if (row == this->nRows) {
                    // All zeroes in this column.
                    doneCol++;
                    continue;
                }
                // Move the non-zero to the top.
                if (row != doneRow) {
                    ans->swapRows(row, doneRow);
                    NMatrixRing<T>::swapRows(row, doneRow);
                }

                // Make the non-zero entry one.
                if (! (this->data[doneRow][doneCol] == NMatrixRing<T>::one)) {
                    ans->divRow(doneRow, this->data[doneRow][doneCol]);
                    divRow(doneRow, this->data[doneRow][doneCol]);
                }

                // Make every other entry in this column zero.
                for (row = 0; row < this->nRows; row++) {
                    if (row == doneRow)
                        continue;
                    if (! (this->data[row][doneCol] == NMatrixRing<T>::zero)) {
                        ans->addRow(doneRow, row, -(this->data[row][doneCol]));
                        addRow(doneRow, row, -(this->data[row][doneCol]));
                    }
                }

                doneCol++;
                doneRow++;
            }
            return ans;
        }
};

/*@}*/

} // namespace regina

#endif

