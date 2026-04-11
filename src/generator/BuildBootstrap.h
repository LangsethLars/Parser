#pragma once

#include "BuildNFA.h"
#include "BuildCFG.h"



class BuildBootstrap {

public:

	BuildBootstrap() :
		m_MainNFA(),
		m_MainCFG(m_MainNFA.m_TokenClasses)
	{
	}

	bool bootstrapLexerAndParser();

private:

	bool bootstrapLexer();
	bool bootstrapParser();

	BuildNFA m_MainNFA;
	BuildCFG m_MainCFG;

};

