
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Computational Engine                                                  *
 *                                                                        *
 *  Copyright (c) 1999-2002, Ben Burton                                   *
 *  For further details contact Ben Burton (benb@acm.org).                *
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
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,        *
 *  MA 02111-1307, USA.                                                   *
 *                                                                        *
 **************************************************************************/

/* end stub */

/*! \file nscript.h
 *  \brief Contains a packet representing a script.
 */

#ifndef __NSCRIPT_H
#ifndef __DOXYGEN
#define __NSCRIPT_H
#endif

#include <map>
#include <utility>
#include <vector>
#include "packet/npacket.h"
#include "property/npropertyholder.h"

/**
 * A packet representing a script that can be run.
 * Accessor methods for a script work a line at a time.
 */
class NScript : public NPacket, public NPropertyHolder {
    private:
        std::vector<NString> lines;
            /**< An array storing the lines of this script; none of
             *   these strings should contain newlines. */
        std::map<NString, NString> variables;
            /**< A map storing the variables with which this script
             *   is to be run.  Variable names are mapped to their
             *   corresponding values. */

    public:
        static const int packetType;

        /**
         * Initialises to a script with no lines and no variables.
         */
        NScript();

        /**
         * Returns the number of lines in this script.
         *
         * @return the number of lines.
         */
        unsigned long getNumberOfLines() const;
        /**
         * Returns the requested line of this script.
         *
         * @param index the index of the requested line; this must be
         * between 0 and getNumberOfLines()-1 inclusive.
         * @return the requested line.
         */
        const NString& getLine(unsigned long index) const;

        /**
         * Adds the given line to the beginning of this script.
         *
         * @param line the line to insert.
         */
        void addFirst(const NString& line);
        /**
         * Adds the given line to the end of this script.
         *
         * @param line the line to add.
         */
        void addLast(const NString& line);
        /**
         * Inserts the given line into the given position in this script.
         * All subsequent lines will be shifted down to make room.
         *
         * @param line the line to insert.
         * @param index the index at which the new line will be placed;
         * this must be between 0 and getNumberOfLines() inclusive.
         */
        void insertAtPosition(const NString& line, unsigned long index);
        /**
         * Replaces a line of this script with the given line.
         *
         * @param line the line to replace the currently stored line.
         * @param index the index of the line to replace; this must be
         * between 0 and getNumberOfLines()-1 inclusive.
         */
        void replaceAtPosition(const NString& line, unsigned long index);
        /**
         * Removes the requested line from this script.
         *
         * @param index the index of the requested line; this must be
         * between 0 and getNumberOfLines()-1 inclusive.
         */
        void removeLineAt(unsigned long index);
        /**
         * Removes all lines from this script.
         */
        void removeAllLines();

        /**
         * Returns the number of variables associated with this script.
         *
         * @return the number of variables.
         */
        unsigned long getNumberOfVariables() const;
        /**
         * Returns the name of the requested variable associated with
         * this script.
         *
         * @param index the index of the requested variable; this must
         * be between 0 and getNumberOfVariables()-1 inclusive.
         * @return the name of the requested variable.
         */
        const NString& getVariableName(unsigned long index) const;
        /**
         * Returns the value of the requested variable associated with
         * this script.
         *
         * If the value is a packet, the packet label will be returned.
         * If the value is \c null, the empty string will be returned.
         *
         * @param index the index of the requested variable; this must
         * be between 0 and getNumberOfVariables()-1 inclusive.
         * @return the value of the requested variable.
         */
        const NString& getVariableValue(unsigned long index) const;
        /**
         * Returns the value of the variable stored with the given
         * name.  The return strings are as described in
         * getVariableValue(unsigned long).
         *
         * If no variable is stored with the given name, the empty
         * string will be returned.
         *
         * @param name the name of the requested variable; note that
         * names are case sensitive.
         * @return the value of the requested variable.
         */
        const NString& getVariableValue(const NString& name) const;

        /**
         * Adds a new variable to be associated with this script.
         * If a variable with the given name is already stored, this
         * routine will do nothing.
         *
         * @param name the name of the new variable.
         * @param value the value of the new variable, as described in
         * the notes for getVariableValue().
         * @return \c true if the variable was successfully added, or
         * \c false if a variable with the given name was already stored.
         */
        bool addVariable(const NString& name, const NString& value);
        /**
         * Removes the variable stored with the given name.
         * Note that the indices of other variables may change as a
         * result of this action.
         *
         * If no variable is stored with the given name, this routine
         * will do nothing (but waste time!).
         *
         * @param name the name of the variable to remove; note that
         * names are case sensitive.
         */
        void removeVariable(const NString& name);
        /**
         * Removes all variables associated with this script.
         */
        void removeAllVariables();

        virtual int getPacketType() const;
        virtual NString getPacketName() const;

        virtual void writeTextShort(std::ostream& out) const;
        virtual void writeTextLong(std::ostream& out) const;
        virtual void writePacket(NFile& out) const;
        static NScript* readPacket(NFile& in, NPacket* parent);
        virtual bool dependsOnParent() const;
    
    protected:
        virtual NPacket* internalClonePacket(NPacket* parent) const;

        virtual void readIndividualProperty(NFile& infile,
            unsigned propType);
        virtual void initialiseAllProperties();
};

// Inline functions for NScript

inline NScript::NScript() {
    NScript::initialiseAllProperties();
}

inline unsigned long NScript::getNumberOfLines() const {
    return lines.size();
}
inline const NString& NScript::getLine(unsigned long index) const {
    return lines[index];
}

inline void NScript::addFirst(const NString& line) {
    lines.insert(lines.begin(), line);
}
inline void NScript::addLast(const NString& line) {
    lines.push_back(line);
}
inline void NScript::insertAtPosition(const NString& line,
        unsigned long index) {
    lines.insert(lines.begin() + index, line);
}
inline void NScript::replaceAtPosition(const NString& line,
        unsigned long index) {
    lines[index] = line;
}
inline void NScript::removeLineAt(unsigned long index) {
    lines.erase(lines.begin() + index);
}
inline void NScript::removeAllLines() {
    lines.clear();
}

inline unsigned long NScript::getNumberOfVariables() const {
    return variables.size();
}
inline bool NScript::addVariable(const NString& name, const NString& value) {
    return variables.insert(std::make_pair(name, value)).second;
}
inline void NScript::removeVariable(const NString& name) {
    variables.erase(name);
}
inline void NScript::removeAllVariables() {
    variables.clear();
}

inline void NScript::writeTextShort(std::ostream& o) const {
    o << "Script with " << lines.size() << " line";
    if (lines.size() != 1)
        o << 's';
}

inline bool NScript::dependsOnParent() const {
    return false;
}

#endif

