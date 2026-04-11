#include "BuildCfg.h"

#include <fstream>
#include <sstream>
#include <iomanip>



BuildCFG& BuildCFG::addProduction(const char *variableName)
{
	int variableId = -1;

	for (size_t n = 0; n < m_VariableClasses.size(); ++n) {
		if (m_VariableClasses[n].name == variableName) {
			variableId = int(n);
			break;
		}
	}

	if (variableId < 0) {
		variableId = int(m_VariableClasses.size());
		m_VariableClasses.emplace_back(variableName);
		m_ExtendVariables.emplace_back();
	}

	m_Productions.emplace_back();
	m_Productions.back().variableClassId = variableId;
	m_ExtendProductions.emplace_back();

	return *this;
}



BuildCFG& BuildCFG::addTerminal(const char *tokenName)
{
	int tokenId = -1;

	for (size_t n = 0; n < m_TokenClasses.size(); ++n) {
		if (m_TokenClasses[n].name == tokenName) {
			tokenId = int(n);
		}
	}

	if (tokenId < 0) {
		printf("Error: Missing definition of token \"%s\" in production %d\n", tokenName, int(m_Productions.size()));
		m_StatusOk = false;
	} else {
		m_Productions.back().symbols.emplace_back(tokenId);
	}
	
	return *this;
}



BuildCFG& BuildCFG::addVariable(const char *variableName)
{
	int variableId = -1;

	for (size_t n = 0; n < m_VariableClasses.size(); ++n) {
		if (m_VariableClasses[n].name == variableName) {
			variableId = int(n);
			break;
		}
	}

	if (variableId < 0) {
		variableId = int(m_VariableClasses.size());
		m_VariableClasses.emplace_back(variableName);
		m_ExtendVariables.emplace_back();
	}

	m_Productions.back().symbols.emplace_back(-(variableId + 1));

	return *this;
}



bool BuildCFG::setVariableClassToSkip(const char* variableName)
{
	for (size_t n = 0; n < m_VariableClasses.size(); ++n) {
		if (m_VariableClasses[n].name == variableName) {
			m_VariableClasses[n].skip = true;
			return true;
		}
	}

	printf("Error: Set variable \"%s\" as skip failed, missing.\n", variableName);
	m_StatusOk = false;

	return false;
}



bool BuildCFG::buildParseTable()
{
	bool ok = true;

	initBuildParameters();

	while (findEmpty());
	while (findFirst());
	while (findFollow());

	m_ParsingTable.resize(m_VariableClasses.size() * m_TokenClasses.size(), -1);

	for (size_t iProduction = 0; iProduction < m_Productions.size(); ++iProduction) {
		int variableId = m_Productions[iProduction].variableClassId;
		ExtendProduction &production = m_ExtendProductions[iProduction];
		size_t iTable = variableId * m_TokenClasses.size();
		for (size_t iTerminal = 0; iTerminal < m_TokenClasses.size(); ++iTerminal, ++iTable) {
			if (
				production.m_FirstTerminals.isBitSet(iTerminal) ||
				(
					production.m_CanBeEmpty &&
					m_ExtendVariables[variableId].m_FollowTerminals.isBitSet(iTerminal)
					)
			) {
				if (m_ParsingTable[iTable] < 0) {
					m_ParsingTable[iTable] = iProduction;
				}
				else if (m_ParsingTable[iTable] != iProduction) {
					printf("Error in production %d for terminal %d first\n", int(iProduction), int(iTerminal));
					printf("Variable %s on token %s already points to production %d\n", m_VariableClasses[variableId].name.c_str(), m_TokenClasses[iTerminal].name.c_str(), m_ParsingTable[iTable]);
					ok = false;
				}
			}
		}
	}

	return ok;
}


void BuildCFG::initBuildParameters()
{

	auto iterVariable = m_ExtendVariables.begin();
	for (; iterVariable != m_ExtendVariables.end(); ++iterVariable) {
		iterVariable->m_CanBeEmpty = false;
		iterVariable->m_FirstTerminals.initBitSet(m_TokenClasses.size());
		iterVariable->m_FollowTerminals.initBitSet(m_TokenClasses.size());
	}

	auto iterProduction = m_ExtendProductions.begin();
	for (; iterProduction != m_ExtendProductions.end(); ++iterProduction) {
		iterProduction->m_CanBeEmpty = false;
		iterProduction->m_FirstTerminals.initBitSet(m_TokenClasses.size());
	}

}



bool BuildCFG::findEmpty()
{
	bool hasFoundNewEmpty = false;

	// If a production is empty or only contain variables that can be empty, then the head variable can be empty
	for (size_t iProduction = 0; iProduction < m_Productions.size(); ++iProduction) {

		bool empty = true;
		Production &production = m_Productions[iProduction];
		ExtendProduction &extendProduction = m_ExtendProductions[iProduction];
		int variableId = production.variableClassId;

		for (auto iterSymbol = production.symbols.begin(); iterSymbol != production.symbols.end(); ++iterSymbol) {
			if ((*iterSymbol >= 0) || (*iterSymbol < 0 && !m_ExtendVariables[-1 - *iterSymbol].m_CanBeEmpty)) {
				empty = false;
				break;
			}
		}

		if (empty) {

			if (!extendProduction.m_CanBeEmpty)
				extendProduction.m_CanBeEmpty = hasFoundNewEmpty = true;

			if (!m_ExtendVariables[variableId].m_CanBeEmpty)
				m_ExtendVariables[variableId].m_CanBeEmpty = hasFoundNewEmpty = true;

		}

	}

	// This function is called until no more empty variables are found
	return hasFoundNewEmpty;
}



bool BuildCFG::findFirst()
{
	bool hasFoundNewFirst = false;

	for (size_t iProduction = 0; iProduction < m_Productions.size(); ++iProduction) {

		Production &production = m_Productions[iProduction];
		ExtendProduction &extendProduction = m_ExtendProductions[iProduction];
		ExtendVariable &productionHead = m_ExtendVariables[m_Productions[iProduction].variableClassId];

		for (auto iterSymbol = production.symbols.begin(); iterSymbol != production.symbols.end(); ++iterSymbol) {

			if ((*iterSymbol >= 0)) {

				if (!extendProduction.m_FirstTerminals.isBitSet(*iterSymbol)) {
					extendProduction.m_FirstTerminals.setBit(*iterSymbol);
					hasFoundNewFirst = true;
				}

				if (!productionHead.m_FirstTerminals.isBitSet(*iterSymbol)) {
					productionHead.m_FirstTerminals.setBit(*iterSymbol);
					hasFoundNewFirst = true;
				}

				break;

			}

			ExtendVariable &symbolVariable = m_ExtendVariables[-1 - *iterSymbol];
			hasFoundNewFirst = productionHead.m_FirstTerminals.orWith(symbolVariable.m_FirstTerminals) || hasFoundNewFirst;
			hasFoundNewFirst = extendProduction.m_FirstTerminals.orWith(symbolVariable.m_FirstTerminals) || hasFoundNewFirst;
			if (!symbolVariable.m_CanBeEmpty)
				break;

		}

	}

	// This function is called until no more first terminals are found
	return hasFoundNewFirst;
}



bool BuildCFG::findFollow()
{
	bool hasFoundNewFollow = false;

	for (size_t iProduction = 0; iProduction < m_Productions.size(); ++iProduction) {

		Production &production = m_Productions[iProduction];

		auto &symbols = production.symbols;
		for (size_t iSymbol = 0; iSymbol < symbols.size(); ++iSymbol) {

			if (symbols[iSymbol] >= 0)
				continue;

			ExtendVariable &variable = m_ExtendVariables[-1 - symbols[iSymbol]];
			size_t iFollow = iSymbol + 1;
			for (; iFollow < symbols.size(); ++iFollow) {

				if (symbols[iFollow] >= 0) {

					if (!variable.m_FollowTerminals.isBitSet(symbols[iFollow])) {
						variable.m_FollowTerminals.setBit(symbols[iFollow]);
						hasFoundNewFollow = true;
					}

					break;

				}
				else {

					ExtendVariable &variableFollow = m_ExtendVariables[-1 - symbols[iFollow]];
					hasFoundNewFollow = variable.m_FollowTerminals.orWith(variableFollow.m_FirstTerminals) || hasFoundNewFollow;
					if (!variableFollow.m_CanBeEmpty)
						break;

				}

			}

			if (iFollow >= symbols.size()) {
				ExtendVariable &productionHead = m_ExtendVariables[m_Productions[iProduction].variableClassId];
				hasFoundNewFollow = variable.m_FollowTerminals.orWith(productionHead.m_FollowTerminals) || hasFoundNewFollow;
			}

		}

	}

	// This function is called until no more follow terminals are found
	return hasFoundNewFollow;
}



bool
BuildCFG::saveCFG(const char* rootName)
{
	int id = 0;

	std::string classNameLexer = rootName;
	classNameLexer += "_Lexer";

	std::string classNameParser = rootName;
	classNameParser += "_Parser";

	std::string nsName = "generated/";
	nsName += classNameParser;

	std::string hFilename = nsName + ".h";
	std::string cppFilename = nsName + ".cpp";

	// Build header file content
	std::ostringstream hFile;
	hFile << "#pragma once\n";
	hFile << "\n";
	hFile << "#include \"ParserBase.h\"\n";
	hFile << "#include \"" << classNameLexer << ".h\"\n";
	hFile << "\n";
	hFile << "class " << classNameParser << " : public ::ParserBase {\n";
	hFile << "\n";
	hFile << "public:\n";
	hFile << "\n";
	hFile << "    " << classNameParser << "() : ::ParserBase(\n";
	hFile << "        g_VariableClasses,\n";
	hFile << "        g_Productions,\n";
	hFile << "        g_ParsingTable,\n";
	hFile << "        " << classNameLexer << "::g_TokenClasses,\n";
	hFile << "        m_" << classNameLexer << ".m_TokenSequence,\n";
	hFile << "        m_" << classNameLexer << ".m_RawText\n";
	hFile << "    ) {}\n";
	hFile << "\n";
	hFile << "    bool lexAndParseFile(const char* filename);\n";
	hFile << "\n";
	hFile << "    " << classNameLexer << " m_" << classNameLexer << ";\n";
	hFile << "\n";
	hFile << "    static const VariableClasses g_VariableClasses;\n";
	hFile << "    static const Productions g_Productions;\n";
	hFile << "    static const ParsingTable g_ParsingTable;\n";

	// Write VariableId enum
	id = 0;
	hFile << "\n";
	hFile << "    enum class VariableId {\n";
	for (auto it = m_VariableClasses.begin(); it != m_VariableClasses.end(); ++it, ++id) {
		std::string str(it->name);
		str.front() = '_';
		str.back() = '_';
		hFile << "        " << str << " = " << id;
		if (std::next(it, 1) != m_VariableClasses.end()) {
			hFile << ",";
		}
		hFile << "\n";
	}
	hFile << "    }; // End of enum class VariableId\n";

	hFile << "\n";
	hFile << "}; // End of class " << classNameParser << "\n";

	// Write header file
	std::ofstream hOut(hFilename);
	if (!hOut.is_open()) {
		printf("Error: Failed to open file '%s' for writing\n", hFilename.c_str());
		return false;
	}
	hOut << hFile.str();
	if (!hOut.good()) {
		printf("Error: Failed to write to file '%s'\n", hFilename.c_str());
		return false;
	}
	hOut.close();

	// Build cpp file content
	std::ostringstream cppFile;
	cppFile << "#include \"" << classNameParser << ".h\"\n";
	cppFile << "\n\n\n";

	cppFile << "bool " << classNameParser << "::lexAndParseFile(const char* filename) {\n";
	cppFile << "    if (!m_" << classNameLexer << ".lexFile(filename)) {\n";
	cppFile << "        printf(\"" << classNameParser << "::lexAndParseFile failed to lexFile(\\\"%s\\\")\\n\", filename);\n";
	cppFile << "        return false;\n";
	cppFile << "    }\n";
	cppFile << "    return parseTokenSequence();\n";
	cppFile << "}\n";
	cppFile << "\n\n\n";

	// Write VariableClasses array
	id = 0;
	cppFile << "const VariableClasses " << classNameParser << "::g_VariableClasses = {\n";
	for (auto it = m_VariableClasses.begin(); it != m_VariableClasses.end(); ++it, ++id) {
		cppFile << "    {\"" << it->name << "\", " << (it->skip ? "true" : "false") << "}";
		if (std::next(it, 1) != m_VariableClasses.end()) {
			cppFile << ",";
		}
		cppFile << " // [" << id << "]\n";
	}
	cppFile << "}; // End of g_VariableClasses\n";
	cppFile << "\n\n\n";

	// Write Productions array
	id = 0;
	cppFile << "const Productions " << classNameParser << "::g_Productions = {\n";
	for (auto it = m_Productions.begin(); it != m_Productions.end(); ++it, ++id) {
		cppFile << "    { " << it->variableClassId << ", { ";
		for (size_t n = 0; n < it->symbols.size(); ++n) {
			cppFile << it->symbols[n];
			if (n < it->symbols.size() - 1) {
				cppFile << ", ";
			}
		}
		cppFile << " } }";
		if (std::next(it, 1) != m_Productions.end()) {
			cppFile << ",";
		}
		cppFile << " // [" << id << "]  " << m_VariableClasses[it->variableClassId].name << "  ==>";
		for (size_t n = 0; n < it->symbols.size(); ++n) {
			if ( it->symbols[n] < 0)
				cppFile << "  " << m_VariableClasses[-1 - it->symbols[n]].name;
			else
				cppFile << "  " << m_TokenClasses[it->symbols[n]].name;
		}
		cppFile << "\n";
	}
	cppFile << "}; // End of g_Productions\n";
	cppFile << "\n\n\n";

	// Write ParsingTable array
	if (m_ParsingTable.size() != m_VariableClasses.size() * m_TokenClasses.size()) {
		printf("Error: ParsingTable size %d does not match VariableClasses size %d * TokenClasses size %d\n", int(m_ParsingTable.size()), int(m_VariableClasses.size()), int(m_TokenClasses.size()));
		return false;
	}
	id = 0;
	cppFile << "const ParsingTable " << classNameParser << "::g_ParsingTable = {\n";
	cppFile << "//"; // Write token names as comment header for the table
	for (auto it = m_TokenClasses.begin(); it != m_TokenClasses.end(); ++it) {
		// Print 5 characters of the token name for better formatting and
		// left padding with space if the name is shorter than 5 characters.
		std::string tokenName = it->name.substr(0, 5);
		tokenName.insert(tokenName.begin(), 5 - tokenName.size(), ' ');
		cppFile << tokenName;
		if (std::next(it, 1) != m_TokenClasses.end()) {
			cppFile << ",";
		}
	}
	cppFile << "\n";
	for (size_t itVariable = 0; itVariable < m_VariableClasses.size(); ++itVariable) {
		cppFile << "  ";
		for (size_t itToken = 0; itToken < m_TokenClasses.size(); ++itToken) {
			// Left padding with space for better formatting of the table, total of 5 characters for each entry.
			cppFile << std::setw(5) << m_ParsingTable[id++];
			if (itToken < m_TokenClasses.size() - 1) {
				cppFile << ",";
			}
		}
		if (itVariable < m_VariableClasses.size() - 1)
			cppFile << ",";
		else
			cppFile << " "; // No comma for the last line of the table
		cppFile << " // [" << itVariable << "] " << m_VariableClasses[itVariable].name << "\n";
	}
	cppFile << "}; // End of g_ParsingTable\n";

	// Write cpp file
	std::ofstream cppOut(cppFilename);
	if (!cppOut.is_open()) {
		printf("Error: Failed to open file '%s' for writing\n", cppFilename.c_str());
		return false;
	}
	cppOut << cppFile.str();
	if (!cppOut.good()) {
		printf("Error: Failed to write to file '%s'\n", cppFilename.c_str());
		return false;
	}
	cppOut.close();

	return true;
}

