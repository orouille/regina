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

#include "rule_p.h"
#include "definition_p.h"

#include <QString>
#include <cassert>
#include <iostream>

#include "utilities/stringutils.h"
#include "utilities/xmlutils.h"

using namespace KSyntaxHighlighting;

Rule::Rule() :
    m_column(-1),
    m_firstNonSpace(false),
    m_lookAhead(false)
{
}

Rule::~Rule()
{
}

Definition Rule::definition() const
{
    return m_def.definition();
}

void Rule::setDefinition(const Definition &def)
{
    m_def = def;
}

const std::string& Rule::attribute() const
{
    return m_attribute;
}

ContextSwitch Rule::context() const
{
    return m_context;
}

bool Rule::isLookAhead() const
{
    return m_lookAhead;
}

bool Rule::firstNonSpace() const
{
    return m_firstNonSpace;
}

int Rule::requiredColumn() const
{
    return m_column;
}

bool Rule::load(xmlTextReaderPtr reader)
{
    m_attribute = regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"attribute"));
    if (regina::xml::xmlString(xmlTextReaderName(reader)) != "IncludeRules") // IncludeRules uses this with a different semantic
        m_context.parse(regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"context")));
    regina::valueOf(regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"firstNonSpace")), m_firstNonSpace);
    regina::valueOf(regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"lookAhead")), m_lookAhead);
    if (! regina::valueOf(regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"column")), m_column))
        m_column = -1;

    auto result = doLoad(reader);

    if (m_lookAhead && m_context.isStay())
        result = false;

    if (xmlTextReaderIsEmptyElement(reader))
        return result;
    if (xmlTextReaderRead(reader) != 1)
        return result;
    while (true) {
        switch (xmlTextReaderNodeType(reader)) {
            case 1 /* start element */:
            {
                auto rule = Rule::create(regina::xml::xmlString(xmlTextReaderName(reader)));
                if (rule) {
                    rule->setDefinition(m_def.definition());
                    if (rule->load(reader)) {
                        m_subRules.push_back(rule);
                        if (xmlTextReaderRead(reader) != 1)
                            return result;
                    }
                } else {
                    // Skip current element.
                    if (xmlTextReaderNext(reader) != 1)
                        return result;
                }
                break;
            }
            case 15 /* end element */:
                return result;
            default:
                if (xmlTextReaderRead(reader) != 1)
                    return result;
                break;
        }
    }

    return result;
}

void Rule::resolveContext()
{
    m_context.resolve(m_def.definition());
    for (const auto &rule : m_subRules)
        rule->resolveContext();
}

bool Rule::doLoad(xmlTextReaderPtr)
{
    return true;
}

MatchResult Rule::match(const QString &text, int offset)
{
    assert(!text.isEmpty());

    const auto result = doMatch(text, offset);
    if (result.offset() == offset || result.offset() == text.size())
        return result;

    for (const auto &subRule : m_subRules) {
        const auto subResult = subRule->match(text, result.offset());
        if (subResult.offset() > result.offset())
            return MatchResult(subResult.offset());
    }

    return result;
}

Rule::Ptr Rule::create(const std::string& name)
{
    Rule *rule = nullptr;
    if (name == "AnyChar")
        rule = new AnyChar;
    else if (name == "DetectChar")
        rule = new DetectChar;
    else if (name == "Detect2Chars")
        rule = new Detect2Char;
    else if (name == "DetectIdentifier")
        rule = new DetectIdentifier;
    else if (name == "DetectSpaces")
        rule = new DetectSpaces;
    else if (name == "Float")
        rule = new Float;
    else if (name == "Int")
        rule = new Int;
    else if (name == "HlCChar")
        rule = new HlCChar;
    else if (name == "HlCHex")
        rule = new HlCHex;
    else if (name == "HlCOct")
        rule = new HlCOct;
    else if (name == "HlCStringChar")
        rule = new HlCStringChar;
    else if (name == "IncludeRules")
        rule = new IncludeRules;
    else if (name == "keyword")
        rule = new KeywordListRule;
    else if (name == "LineContinue")
        rule = new LineContinue;
    else if (name == "RangeDetect")
        rule = new RangeDetect;
    else if (name == "RegExpr")
        rule = new RegExpr;
    else if (name == "StringDetect")
        rule = new StringDetect;
    else if (name == "WordDetect")
        rule = new WordDetect;
    else
        std::cerr << "Unknown rule type: " << name << std::endl;

    return Ptr(rule);
}

bool Rule::isDelimiter(QChar c) const
{
    auto defData = DefinitionData::get(m_def.definition());
    return defData->isDelimiter(c.toLatin1());
}


bool AnyChar::doLoad(xmlTextReaderPtr reader)
{
    m_chars = regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"String"));
    if (m_chars.length() == 1)
        std::cerr << "AnyChar rule with just one char: use DetectChar instead." << std::endl;
    return !m_chars.empty();
}

bool DetectChar::doLoad(xmlTextReaderPtr reader)
{
    const auto s = regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"char"));
    if (s.empty())
        return false;
    m_char = s.front();
    return true;
}

bool Detect2Char::doLoad(xmlTextReaderPtr reader)
{
    const auto s1 = regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"char"));
    const auto s2 = regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"char1"));
    if (s1.empty() || s2.empty())
        return false;
    m_char1 = s1.front();
    m_char2 = s2.front();
    return true;
}

const std::string& IncludeRules::contextName() const
{
    return m_contextName;
}

const std::string& IncludeRules::definitionName() const
{
    return m_defName;
}

bool IncludeRules::includeAttribute() const
{
    return m_includeAttribute;
}

bool IncludeRules::doLoad(xmlTextReaderPtr reader)
{
    const auto s = regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"context"));
    if (s.empty())
        return false;

    auto pos = s.find("##");
    if (pos != std::string::npos) {
        m_contextName = s.substr(0, pos);
        m_defName = s.substr(pos + 2);
    } else {
        m_contextName = s;
    }

    regina::valueOf(regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"includeAttrib")), m_includeAttribute);

    return !m_contextName.empty() || !m_defName.empty();
}


bool KeywordListRule::doLoad(xmlTextReaderPtr reader)
{
    m_listName = regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"String"));

    auto attr = xmlTextReaderGetAttribute(reader, (const xmlChar*)"insensitive");
    if (attr) {
        bool cs;
        regina::valueOf(regina::xml::xmlString(attr), cs);
        m_caseSensitivityOverride = ! cs;
        m_hasCaseSensitivityOverride = true;
    } else {
        m_hasCaseSensitivityOverride = false;
    }
    return !m_listName.empty();
}

bool LineContinue::doLoad(xmlTextReaderPtr reader)
{
    const auto s = regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"char"));
    if (s.empty())
        m_char = QLatin1Char('\\');
    else
        m_char = s.front();
    return true;
}

bool RangeDetect::doLoad(xmlTextReaderPtr reader)
{
    const auto s1 = regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"char"));
    const auto s2 = regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"char1"));
    if (s1.empty() || s2.empty())
        return false;
    m_begin = s1.front();
    m_end = s2.front();
    return true;
}

bool RegExpr::doLoad(xmlTextReaderPtr reader)
{
    m_pattern = QString::fromUtf8(
        regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"String")).c_str());
    m_regexp.setPattern(m_pattern);
    bool isMinimal, isCaseInsensitive;
    regina::valueOf(regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"minimal")), isMinimal);
    regina::valueOf(regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"insensitive")), isCaseInsensitive);
    m_regexp.setPatternOptions(
        (isMinimal ? QRegularExpression::InvertedGreedinessOption : QRegularExpression::NoPatternOption) |
        (isCaseInsensitive ? QRegularExpression::CaseInsensitiveOption : QRegularExpression::NoPatternOption));
    return !m_pattern.isEmpty(); // m_regexp.isValid() would be better, but parses the regexp and thus is way too expensive
}

bool StringDetect::doLoad(xmlTextReaderPtr reader)
{
    m_string = regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"String"));

    bool cs;
    regina::valueOf(regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"insensitive")), cs);
    m_caseSensitivity = ! cs;

    return !m_string.empty();
}

bool WordDetect::doLoad(xmlTextReaderPtr reader)
{
    m_word = regina::xml::xmlString(xmlTextReaderGetAttribute(reader, (const xmlChar*)"String"));
    return !m_word.empty();
}
