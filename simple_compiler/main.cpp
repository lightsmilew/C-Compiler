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
int main(int argc, char* argv[]) {
    std::string filename; //文件名
    bool runLexer = false;
    bool runParser = false;
    bool runAssembler = false;
    // 解析命令行参数
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
                filename = argv[++i]; // 更新文件名为下一个参数
            }
            else {
                std::cerr << "错误: 文件名未提供。\n";
                return 1;
            }
        }
        else if (arg == "-h" || arg == "--help") {
            show_help();
            break;
        }
        else {
            std::cerr << "未知参数: " << arg << "\n";
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
            std::vector<std::string> args; // 程序名作为第一个参数
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
                            filename = args[++i]; // 更新文件名为下一个参数
                        }
                        else {
                            std::cout << "错误: 文件名未提供" << std::endl;
                        }
                    }
                    else if (arg == "-h" || arg == "--help") {
                        show_help();
                        break;
                    }
                    else {
                        std::cout << "未知参数: " << arg << std::endl;
                    }
                }
            }
            else {
                std::cout << "命令头缺少compiler" << endl;
            }
        }
    }
    // 运行对应模块
    if (runLexer) {
        std::cout << "运行词法分析器于文件: " << filename << std::endl;
        Lexer lexer(filename);
        lexer.main();
        lexer.generateTokens();
        //lexer.display();调试用
    }
    if (runParser) {
        std::cout << "运行语法分析器于文件: " << filename << std::endl;
        Parser parser(filename);
        parser.main();
        //parser.display();
    }
    if (runAssembler) {
        std::cout << "运行汇编器于文件: " << filename << std::endl;
        Assembler assembler(filename);
        assembler.main();
    }
    return 0;
}