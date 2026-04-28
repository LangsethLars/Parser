#include "ParseTreeIterator.h"
#include <cstdio>



// Construction
// ====================================

ParseTreeIterator::ParseTreeIterator(
    const ParseTree& parseTree, 
    const TokenSequence& tokenSequence, 
    const RawText& rawText, 
    int startNode)
    : m_ParseTree(parseTree)
    , m_TokenSequence(tokenSequence)
    , m_RawText(rawText)
    , m_NodeIndex(startNode)
{
    validateIndex();
}



// Navigation - return new iterator
// ====================================

ParseTreeIterator ParseTreeIterator::parent() const {
    validateNotRoot();
    return ParseTreeIterator(m_ParseTree, m_TokenSequence, m_RawText, getNode().parent);
}

ParseTreeIterator ParseTreeIterator::firstChild() const {
    validateHasChildren();
    return ParseTreeIterator(m_ParseTree, m_TokenSequence, m_RawText, getNode().firstChild);
}

ParseTreeIterator ParseTreeIterator::lastChild() const {
    validateHasChildren();
    return ParseTreeIterator(m_ParseTree, m_TokenSequence, m_RawText, getNode().lastChild);
}

ParseTreeIterator ParseTreeIterator::nextSibling() const {
    validateHasSibling(true);
    return ParseTreeIterator(m_ParseTree, m_TokenSequence, m_RawText, getNode().next);
}

ParseTreeIterator ParseTreeIterator::prevSibling() const {
    validateHasSibling(false);
    return ParseTreeIterator(m_ParseTree, m_TokenSequence, m_RawText, getNode().prev);
}



// Navigation - mutate this iterator
// ====================================

ParseTreeIterator& ParseTreeIterator::moveToParent() {
    validateNotRoot();
    m_NodeIndex = getNode().parent;
    return *this;
}

ParseTreeIterator& ParseTreeIterator::moveToFirstChild() {
    validateHasChildren();
    m_NodeIndex = getNode().firstChild;
    return *this;
}

ParseTreeIterator& ParseTreeIterator::moveToLastChild() {
    validateHasChildren();
    m_NodeIndex = getNode().lastChild;
    return *this;
}

ParseTreeIterator& ParseTreeIterator::moveToNextSibling() {
    validateHasSibling(true);
    m_NodeIndex = getNode().next;
    return *this;
}

ParseTreeIterator& ParseTreeIterator::moveToPrevSibling() {
    validateHasSibling(false);
    m_NodeIndex = getNode().prev;
    return *this;
}



// Symbol information
// ====================================

int ParseTreeIterator::getTokenIdNoThrow() const {
    if (isValid()) {
        int tokenNo = m_ParseTree[m_NodeIndex].symbolIdOrTokenSequenceNo;
        if (tokenNo >= 0 && tokenNo < static_cast<int>(m_TokenSequence.size())) {
            return m_TokenSequence[tokenNo].tokenClassId;
        }
    }
    return -1;
}

int ParseTreeIterator::getVariableIdNoThrow() const {
    if (isValid()) {
        int variableId = -1 - m_ParseTree[m_NodeIndex].symbolIdOrTokenSequenceNo;
        if (variableId >= 0) {
			return variableId; //  Check against g_VariableClasses.size() if needed
        }
    }
    return -1;
}



// Token access
// ====================================

bool ParseTreeIterator::isTerminal() const {
    return getNode().symbolIdOrTokenSequenceNo >= 0;
}

bool ParseTreeIterator::isNonTerminal() const {
    return getNode().symbolIdOrTokenSequenceNo < 0;
}

const Token& ParseTreeIterator::getToken() const {
    validateIsTerminal();
    int tokenIndex = getNode().symbolIdOrTokenSequenceNo;
    if (tokenIndex < 0 || tokenIndex >= static_cast<int>(m_TokenSequence.size())) {
        throw std::runtime_error("ParseTreeIterator: Token index out of bounds");
    }
    return m_TokenSequence[tokenIndex];
}

std::string ParseTreeIterator::getLexeme() const {
    const Token& token = getToken();
    if (token.lexemeStart >= m_RawText.size()) {
        throw std::runtime_error("ParseTreeIterator: Lexeme start position out of bounds");
    }
    size_t endPos = token.lexemeStart + token.lexemeLength;
    if (endPos > m_RawText.size()) {
        throw std::runtime_error("ParseTreeIterator: Lexeme end position out of bounds");
    }
    return std::string(
        reinterpret_cast<const char*>(&m_RawText[token.lexemeStart]), 
        token.lexemeLength
    );
}



// Iteration helpers
// ====================================

ParseTreeIterator& ParseTreeIterator::operator++() {
    moveToNextSibling();
    return *this;
}

ParseTreeIterator ParseTreeIterator::operator++(int) {
    ParseTreeIterator temp(*this);
    moveToNextSibling();
    return temp;
}



// Comparison
// ====================================

bool ParseTreeIterator::operator==(const ParseTreeIterator& rhs) const {
    return (&m_ParseTree == &rhs.m_ParseTree) && (m_NodeIndex == rhs.m_NodeIndex);
}

bool ParseTreeIterator::operator!=(const ParseTreeIterator& rhs) const {
    return !(*this == rhs);
}



// Debugging
// ====================================

void ParseTreeIterator::debug(const char* message) const {
    if (message) {
        printf("ParseTreeIterator Debug: %s\n", message);
    }
    printf("  Node Index: %d\n", m_NodeIndex);
    if (isValid()) {
        const ParseNode& node = getNode();
        printf("  Symbol/Token: %d\n", node.symbolIdOrTokenSequenceNo);
        printf("  Parent: %d, FirstChild: %d, LastChild: %d\n", 
            node.parent, node.firstChild, node.lastChild);
        printf("  Prev: %d, Next: %d\n", node.prev, node.next);
        printf("  Type: %s\n", isTerminal() ? "Terminal" : "NonTerminal");
        if (isTerminal()) {
            try {
                printf("  Lexeme: \"%s\"\n", getLexeme().c_str());
            } catch (...) {
                printf("  Lexeme: <error>\n");
            }
        }
    } else {
        printf("  <Invalid node index>\n");
    }
}



// Validation helpers
// ====================================

void ParseTreeIterator::validateIndex() const {
    if (!isValid()) {
        throw std::runtime_error("ParseTreeIterator: Invalid node index " + std::to_string(m_NodeIndex));
    }
}

void ParseTreeIterator::validateNotRoot() const {
    validateIndex();
    if (!hasParent()) {
        throw std::runtime_error("ParseTreeIterator: Cannot navigate to parent - already at root");
    }
}

void ParseTreeIterator::validateHasChildren() const {
	int firstChild = getNode().firstChild;
	int lastChild = getNode().lastChild;
    if (firstChild < 0) {
        throw std::runtime_error("ParseTreeIterator: Node has no first child");
	}
    else if (lastChild < 0) {
        throw std::runtime_error("ParseTreeIterator: Node has no last child");
	}
    else if (firstChild > lastChild) {
        throw std::runtime_error("ParseTreeIterator: Invalid child indices - firstChild > lastChild");
    }
    else if (firstChild <= m_NodeIndex) {
        throw std::runtime_error("ParseTreeIterator: Invalid child index - firstChild <= current node index");
    }
    else if (firstChild >= static_cast<int>(m_ParseTree.size())) {
		throw std::runtime_error("ParseTreeIterator: Invalid child index - firstChild out of bounds");
	}
    else if (lastChild <= m_NodeIndex) {
        throw std::runtime_error("ParseTreeIterator: Invalid child index - lastChild <= current node index");
    }
    else if (lastChild >= static_cast<int>(m_ParseTree.size())) {
        throw std::runtime_error("ParseTreeIterator: Invalid child index - lastChild out of bounds");
    }
}

void ParseTreeIterator::validateHasSibling(bool next) const {
    if (next) {
		int nextSibling = getNode().next;
        if (nextSibling < 0) {
            throw std::runtime_error("ParseTreeIterator: No next sibling");
        }
        else if (nextSibling <= m_NodeIndex) {
            throw std::runtime_error("ParseTreeIterator: Invalid next sibling index - nextSibling <= current node index");
        }
        else if (nextSibling >= static_cast<int>(m_ParseTree.size())) {
            throw std::runtime_error("ParseTreeIterator: Next sibling index out of bounds");
        }
    } else {
        int prevSibling = getNode().prev;
        if (prevSibling < 0) {
            throw std::runtime_error("ParseTreeIterator: No previous sibling");
        }
        else if (prevSibling >= m_NodeIndex) {
            throw std::runtime_error("ParseTreeIterator: Invalid previous sibling index - prevSibling >= current node index");
        }
    }
}

void ParseTreeIterator::validateIsTerminal() const {
    validateIndex();
    if (!isTerminal()) {
        throw std::runtime_error("ParseTreeIterator: Node is not a terminal - cannot access token data");
    }
}