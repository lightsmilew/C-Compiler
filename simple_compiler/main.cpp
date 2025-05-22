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
    std::string filename = "source.c"; // 默认文件名
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
        else {
            std::cerr << "未知参数: " << arg << "\n";
            std::cerr << "用法: " << argv[0] << " [-l | -p | -a] [-f filename]\n";
            return 1;
        }
    }
    // 如果没有指定任何模块，则默认运行 assembler
    if (!runLexer && !runParser && !runAssembler) {
        runAssembler = true;
    }
    // 运行对应模块
    if (runLexer) {
        std::cout << "运行词法分析器于文件: " << filename << std::endl;
        Lexer lexer(filename);
        lexer.main();
        lexer.generateTokens();
        //lexer.display();调试用
        //输出文件

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