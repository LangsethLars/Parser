#pragma once

#include "ParserTypes.h"
#include <stdexcept>
#include <string>



class ParseTreeIterator {

public:

    // Construction
    ParseTreeIterator(const ParseTree& parseTree, const TokenSequence& tokenSequence, const RawText& rawText, int startNode = 0);
    ParseTreeIterator(const ParseTreeIterator& rhs) = default;
    ParseTreeIterator& operator=(const ParseTreeIterator& rhs) = default;

    // Navigation - return new iterator (non-mutating)
    ParseTreeIterator parent() const;
    ParseTreeIterator firstChild() const;
    ParseTreeIterator lastChild() const;
    ParseTreeIterator nextSibling() const;
    ParseTreeIterator prevSibling() const;

    // Navigation - mutate this iterator (returns *this for chaining)
    ParseTreeIterator& moveToParent();
    ParseTreeIterator& moveToFirstChild();
    ParseTreeIterator& moveToLastChild();
    ParseTreeIterator& moveToNextSibling();
    ParseTreeIterator& moveToPrevSibling();

	// Query current node, only getNode() will throw if the node index is invalid.
    bool isValid()             const { return m_NodeIndex >= 0 && m_NodeIndex < static_cast<int>(m_ParseTree.size()); }
    bool hasParent()           const { return getNode().parent >= 0; }
    bool hasChildren()         const { return getNode().firstChild >= 0; }
    bool hasNextSibling()      const { return getNode().next >= 0; }
    bool hasPrevSibling()      const { return getNode().prev >= 0; }
    int getNodeIndex()         const { return m_NodeIndex; }
    const ParseNode& getNode() const { validateIndex(); return m_ParseTree[m_NodeIndex]; }

    // Symbol information
    int getTokenIdNoThrow()    const;  // Returns: positive for token id, -1 for variable or error
    int getVariableIdNoThrow() const;  // Returns: positive for variable id, -1 for token or error

    // Token access (only valid if isTerminal())
    bool isTerminal() const;            // Is this a token/terminal?
    bool isNonTerminal() const;         // Is this a variable/non-terminal?
    const Token& getToken() const;
    std::string getLexeme() const;

    // Iteration helpers
    ParseTreeIterator& operator++();      // Move to next sibling (prefix)
    ParseTreeIterator operator++(int);    // Move to next sibling (postfix)

    // Comparison
    bool operator==(const ParseTreeIterator& rhs) const;
    bool operator!=(const ParseTreeIterator& rhs) const;

    // Debugging
    void debug(const char* message = nullptr) const;

private:

    // Helper methods for validation
    void validateIndex() const;
    void validateNotRoot() const;
    void validateHasChildren() const;
    void validateHasSibling(bool next) const;
    void validateIsTerminal() const;

    // Data members
    const ParseTree& m_ParseTree;
    const TokenSequence& m_TokenSequence;
    const RawText& m_RawText;
    int m_NodeIndex;

};