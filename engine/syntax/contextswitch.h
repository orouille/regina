/*
    Copyright (C) 2016 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __SYNTAX_CONTEXTSWITCH_H
#ifndef __DOXYGEN
#define __SYNTAX_CONTEXTSWITCH_H
#endif

#include <string>
#include <boost/noncopyable.hpp>

namespace regina {
namespace syntax {

class Context;
class Definition;

class ContextSwitch : public boost::noncopyable
{
public:
    ContextSwitch();
    ~ContextSwitch();

    bool isStay() const;

    int popCount() const;
    Context* context() const;

    void parse(const std::string& contextInstr);
    void resolve(const Definition &def);

private:
    std::string m_defName;
    std::string m_contextName;
    Context *m_context;
    int m_popCount;
};

} } // namespace regina::syntax

#endif