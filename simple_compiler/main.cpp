#include<iostream>
#include"Lexer.h"
#include"Parser.h"
#include"Assembler.h"
#include<sstream>
using namespace std;
void show_help() {
    std::cout << "ʹ��˵��:\n"
        << "[-l | --lexer]      ���дʷ�������\n"
        << "[-p | --parser]     �����﷨������\n"
        << "[-a | --assembler]  ���л����\n"
        << "[-f | --file]       ָ��Դ�ļ�·��\n"
        << "[-h | --help]       ��ʾ�˰�����Ϣ\n"
        << "[-e | --exit]       �˳�����\n"
        << "ʾ��: \n"
        << "compiler -l -f source.c\n"
        << "compiler -e\n";
}
int main(int argc, char* argv[]) {
    std::string filename; //�ļ���
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
        else if (arg == "-h" || arg == "--help") {
            show_help();
            break;
        }
        else {
            std::cerr << "δ֪����: " << arg << "\n";
            return 1;
        }
    }
    if (!runLexer && !runParser && !runAssembler) {
        bool is_break = false;
        show_help();
        while (!is_break) {
            runLexer = runParser = runAssembler = false;
            filename = "";
            std::string input;
            std::getline(std::cin, input);
            if (input.empty())continue;
            std::istringstream iss(input);
            std::vector<std::string> args; // ��������Ϊ��һ������
            std::string arg;
            while (iss >> arg) {
                args.push_back(arg);
            }
            if (args[0] == "compiler") {
                for (int i = 1; i < args.size(); i++) {
                    std::string arg = args[i];
                    if ((arg == "-l" || arg == "--lexer") && !runParser && !runAssembler) {
                        runLexer = true;
                        is_break = true;
                    }
                    else if ((arg == "-p" || arg == "--parser") && !runLexer && !runAssembler) {
                        runParser = true;
                        is_break = true;
                    }
                    else if ((arg == "-a" || arg == "--assembler") && !runLexer && !runParser) {
                        runAssembler = true;
                        is_break = true;
                    }
                    else if ((arg == "-e" || arg == "--exit") && !runLexer && !runParser) {
                        is_break = true;
                    }
                    else if (arg == "-f" || arg == "--file") {
                        if (i + 1 < args.size()) {
                            filename = args[++i]; // �����ļ���Ϊ��һ������
                        }
                        else {
                            std::cout << "����: �ļ���δ�ṩ" << std::endl;
                        }
                    }
                    else if (arg == "-h" || arg == "--help") {
                        show_help();
                        break;
                    }
                    else {
                        std::cout << "δ֪����: " << arg << std::endl;
                    }
                }
            }
            else {
                std::cout << "����ͷȱ��compiler" << endl;
            }
        }
    }
    // ���ж�Ӧģ��
    if (runLexer) {
        std::cout << "���дʷ����������ļ�: " << filename << std::endl;
        Lexer lexer(filename);
        lexer.main();
        lexer.generateTokens();
        //lexer.display();������
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