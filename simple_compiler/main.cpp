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
int main() {
    std::string filename="source.c"; //�ļ���
    int runMode = -1;
    //1 2 3�ֱ����Lexer��Parser��Assembler
    bool is_break = false;
    show_help();
        while (!is_break) {
            runMode = -1;
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
                    if ((arg == "-l" || arg == "--lexer") && runMode==-1) {
                        runMode = 1;
                    }
                    else if ((arg == "-p" || arg == "--parser") && runMode == -1) {
                        runMode = 2;
                    }
                    else if ((arg == "-a" || arg == "--assembler") && runMode == -1) {
                        runMode = 3;
                    }
                    else if ((arg == "-e" || arg == "--exit") && runMode == -1) {
                        is_break = true;
                    }
                    else if (arg == "-f" || arg == "--file") {
                        if (i + 1 < args.size()) {
                            filename = args[++i]; // �����ļ���Ϊ��һ������
                        }
                        else {
                            runMode = -1;
                            std::cout << "����: �ļ���δ�ṩ" << std::endl;
                            break;
                        }
                    }
                    else if (arg == "-h" || arg == "--help") {
                        show_help();
                        break;
                    }
                    else {
                        runMode = -1;
                        std::cout << "δ֪����: " << arg << std::endl;
                        break;
                    }
                }
                // ���ж�Ӧģ��
                if (runMode ==1) {
                    std::cout << "���дʷ����������ļ�: " << filename << std::endl;
                    Lexer lexer(filename);
                    lexer.main();
                    lexer.generateTokens();
                }
                if (runMode == 2) {
                    std::cout << "�����﷨���������ļ�: " << filename << std::endl;
                    Parser parser(filename);
                    parser.main();
                }
                if (runMode == 3) {
                    std::cout << "���л�������ļ�: " << filename << std::endl;
                    Assembler assembler(filename);
                    assembler.main();
                }
            }
            else {
                std::cout << "����ͷȱ��compiler" << endl;
            }
        }
    return 0;
}