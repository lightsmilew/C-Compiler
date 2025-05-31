#include<iostream>
#include"Lexer.h"
#include"Parser.h"
#include"Assembler.h"
#include<sstream>
using namespace std;
void show_help() {
    std::cout << "使用说明:\n"
        << "[-l | --lexer]      运行词法分析器\n"
        << "[-p | --parser]     运行语法分析器\n"
        << "[-a | --assembler]  运行汇编器\n"
        << "[-f | --file]       指定源文件路径\n"
        << "[-h | --help]       显示此帮助信息\n"
        << "[-e | --exit]       退出程序\n"
        << "示例: \n"
        << "compiler -l -f source.c\n"
        << "compiler -e\n";
}
int main() {
    std::string filename="source.c"; //文件名
    int runMode = -1;
    //1 2 3分别代表Lexer，Parser，Assembler
    bool is_break = false;
    show_help();
        while (!is_break) {
            runMode = -1;
            filename = "";
            std::string input;
            std::getline(std::cin, input);
            if (input.empty())continue;
            std::istringstream iss(input);
            std::vector<std::string> args; // 程序名作为第一个参数
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
                            filename = args[++i]; // 更新文件名为下一个参数
                        }
                        else {
                            runMode = -1;
                            std::cout << "错误: 文件名未提供" << std::endl;
                            break;
                        }
                    }
                    else if (arg == "-h" || arg == "--help") {
                        show_help();
                        break;
                    }
                    else {
                        runMode = -1;
                        std::cout << "未知参数: " << arg << std::endl;
                        break;
                    }
                }
                // 运行对应模块
                if (runMode ==1) {
                    std::cout << "运行词法分析器于文件: " << filename << std::endl;
                    Lexer lexer(filename);
                    lexer.main();
                    lexer.generateTokens();
                }
                if (runMode == 2) {
                    std::cout << "运行语法分析器于文件: " << filename << std::endl;
                    Parser parser(filename);
                    parser.main();
                }
                if (runMode == 3) {
                    std::cout << "运行汇编器于文件: " << filename << std::endl;
                    Assembler assembler(filename);
                    assembler.main();
                }
            }
            else {
                std::cout << "命令头缺少compiler" << endl;
            }
        }
    return 0;
}