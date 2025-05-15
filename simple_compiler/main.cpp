#include<iostream>
#include"Lexer.h"
#include"Parser.h"
#include"Assembler.h"
using namespace std;
int main() {
	//Lexer lexer("source.c");
	//lexer.main();
	//lexer.display();
	//Parser parser("source.c");
	//parser.main();
	//parser.display();
	Assembler assembler("source.c");
	assembler.main();
	return 0;
}