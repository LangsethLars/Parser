// Main.cpp : Defines the entry point for the console application.
//
// Made by Lars Langseth as part of testing out consepts from two courses on https://www.coursera.org/
// "Automata"  by Jeffrey Ullman
// "Compilers" by Alex Aiken
//
// https://github.com/LangsethLars/Parser

#include "Generator.h"



int main(int argc, char *argv[])
{
	bool ok = false;

	Generator generator;
	//ok = generator.bootstrap();
//	ok = generator.buildTablesFrom("DefaultCFG");
	ok = generator.buildTablesFrom("MetaMathCFG");

	if (ok)
		printf("\nFinished with no errors.\n");
	else
		printf("\nTerminated with ERRORS.\n");

	return 0;
}
