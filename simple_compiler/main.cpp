#include<iostream>
#include"Lexer.h"
#include"Parser.h"
#include"Assembler.h"
using namespace std;
//int main() {
//	//Lexer lexer("source.c");
//	//lexer.main();
//	//lexer.display();
//	//Parser parser("source.c");
//	//parser.main();
//	//parser.display();
//	Assembler assembler("source.c");
//	assembler.main();
//	return 0;
//}
int main(int argc, char* argv[]) {
    std::string filename = "source.c"; // Ĭ���ļ���
    bool runLexer = false;
    bool runParser = false;
    bool runAssembler = false;
    // ���������в���
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-l" || arg == "--lexer") && !runParser && !runAssembler) {
            runLexer = true;
        }
        else if ((arg == "-p" || arg == "--parser") && !runLexer && !runAssembler) {
            runParser = true;
        }
        else if ((arg == "-a" || arg == "--assembler") && !runLexer && !runParser) {
            runAssembler = true;
        }
        else if (arg == "-f" || arg == "--file") {
            if (i + 1 < argc) {
                filename = argv[++i]; // �����ļ���Ϊ��һ������
            }
            else {
                std::cerr << "����: �ļ���δ�ṩ��\n";
                return 1;
            }
        }
        else {
            std::cerr << "δ֪����: " << arg << "\n";
            std::cerr << "�÷�: " << argv[0] << " [-l | -p | -a] [-f filename]\n";
            return 1;
        }
    }
    // ���û��ָ���κ�ģ�飬��Ĭ������ assembler
    if (!runLexer && !runParser && !runAssembler) {
        runAssembler = true;
    }
    // ���ж�Ӧģ��
    if (runLexer) {
        std::cout << "���дʷ����������ļ�: " << filename << std::endl;
        Lexer lexer(filename);
        lexer.main();
        lexer.generateTokens();
        //lexer.display();������
        //����ļ�

    }
    if (runParser) {
        std::cout << "�����﷨���������ļ�: " << filename << std::endl;
        Parser parser(filename);
        parser.main();
        //parser.display();
    }
    if (runAssembler) {
        std::cout << "���л�������ļ�: " << filename << std::endl;
        Assembler assembler(filename);
        assembler.main();
    }
    return 0;
}