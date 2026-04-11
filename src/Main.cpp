// Main.cpp : Defines the entry point for the console application.
//
// Made by Lars Langseth as part of testing out consepts from two courses on https://www.coursera.org/
// "Automata"  by Jeffrey Ullman
// "Compilers" by Alex Aiken
//
// https://github.com/LangsethLars/Parser

//#define BOOTSTRAP
#ifdef BOOTSTRAP

#include "generator/BuildBootstrap.h"

#else

//#define GENERATOR
#ifdef GENERATOR

#include "generator/Generator.h"

#else

#include "parser/Bootstrap_Lexer.h"
#include "parser/Bootstrap_Parser.h"

#endif

#endif

#include <stdio.h>



int main(int argc, char *argv[])
{
	bool ok = false;

#ifdef BOOTSTRAP

	BuildBootstrap generator;
	ok = generator.bootstrapLexerAndParser();

#else

#ifdef GENERATOR

	Generator generator;
	bool bLexerOnly = false;
	ok = generator.makeCodeFromScript("Bootstrap", bLexerOnly);

#else

	Bootstrap_Parser parser;
	ok = parser.lexAndParseFile("scripts/Bootstrap.txt");
	parser.debug();

#endif

#endif

	if (ok)
		printf("\nFinished with no errors.\n");
	else
		printf("\nTerminated with ERRORS.\n");

	return 0;
}
