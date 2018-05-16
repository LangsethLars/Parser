#include "BuildParser.h"



BuildParser &BuildParser::addProduction(const char *variableName)
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



BuildParser &BuildParser::addTerminal(const char *tokenName)
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



BuildParser &BuildParser::addVariable(const char *variableName)
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



bool BuildParser::buildParceTable(ParsingTable &parsingTable)
{
	bool ok = true;

	initBuildParameters();

	while (findEmpty());
	while (findFirst());
	while (findFollow());

	parsingTable.resize(m_VariableClasses.size() * m_TokenClasses.size(), -1);

	for (int iProduction = 0; iProduction < m_Productions.size(); ++iProduction) {
		int variableId = m_Productions[iProduction].variableClassId;
		ExtendProduction &production = m_ExtendProductions[iProduction];
		size_t iTable = variableId * m_TokenClasses.size();
		for (int iTerminal = 0; iTerminal < m_TokenClasses.size(); ++iTerminal, ++iTable) {
			if (
				production.m_FirstTerminals.isBitSet(iTerminal) ||
				(
					production.m_CanBeEmpty &&
					m_ExtendVariables[variableId].m_FollowTerminals.isBitSet(iTerminal)
					)
			) {
				if (parsingTable[iTable] < 0) {
					parsingTable[iTable] = iProduction;
				}
				else if (parsingTable[iTable] != iProduction) {
					printf("Error in production %d for terminal %d first\n", int(iProduction), int(iTerminal));
					printf("Variable %s on token %s already points to production %d\n", m_VariableClasses[variableId].name.c_str(), m_TokenClasses[iTerminal].name.c_str(), parsingTable[iTable]);
					ok = false;
				}
			}
		}
	}

	return ok;
}


void BuildParser::initBuildParameters()
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



bool BuildParser::findEmpty()
{
	bool hasFoundNewEmpty = false;

	// If a production is empty or only contain variables that can be empty, then the head variable can be empty
	for (int iProduction = 0; iProduction < m_Productions.size(); ++iProduction) {

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



bool BuildParser::findFirst()
{
	bool hasFoundNewFirst = false;

	for (int iProduction = 0; iProduction < m_Productions.size(); ++iProduction) {

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



bool BuildParser::findFollow()
{
	bool hasFoundNewFollow = false;

	for (int iProduction = 0; iProduction < m_Productions.size(); ++iProduction) {

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
