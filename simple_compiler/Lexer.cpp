#include"Lexer.h"
#include<fstream>
#include <regex>
#include<iostream>
#include<sstream>
extern map<string, string> map_token_type;
extern vector<vector<string>> keywords;
extern vector<string> operators;
extern vector<char> delimiters;

bool Lexer::is_blank(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

size_t Lexer::skip_blank(size_t i) {
    while (i < content.size() && is_blank(content[i])) {
        ++i;
    }
    return i;
}
bool Lexer::is_keyword(const std::string& value) {
    for (const auto& group : keywords) {
        if (find(group.begin(), group.end(), value) != group.end()) {
            return true;
        }
    }
    return false;
}
bool Lexer::is_delimiters(char c) {
    for (auto p : delimiters) {
        if (p == c)return true;
    }
    return false;
}
bool Lexer::is_operators(string str) {
    for (auto p : operators) {
        if (p == str)return true;
    }
    return false;
}
void Lexer::display() {
    //调试用
    int count = 0;
    for (auto p : tokens) {
        //cout << p.type_n << " : " << p.value << endl;
        cout << p.type_n << " : " << p.value <<":"<<count<< endl;
        count++;
    }
}
void Lexer::generateTokens() {
    if (!is_open_file)return;
    stringstream ss(filename);
    string split_filename;
    getline(ss, split_filename, '.');
    // 打开或创建文件用于写入。
    std::ofstream outFile(split_filename+"_lexer.txt");
    // 检查文件是否成功打开
    if (!outFile.is_open()) {
        std::cerr << "无法打开文件进行写入。" << std::endl;
        return;
    }
    for (auto p : tokens) {
        // 写入到文件中
        outFile << p.type_n << " " << p.value << std::endl;
    }
    // 关闭文件
    outFile.close();
}
void Lexer::main() {
    //读入文件
    std::ifstream fs(filename);
    if (!fs.is_open()) { 
        std::cout << "无法打开文件: " << filename << std::endl;
        return; 
    }
    is_open_file = true;
    while (getline(fs, content)) {
        size_t i = 0;
        while (i < content.size()) {
            i=skip_blank(i);
            //识别头文件
            if (content[i] == '#') {
                tokens.emplace_back(SEPARATOR, content[i]);
                i=skip_blank(i+1);
                while (i < content.size()) {
                    regex includeRegex("include");
                    //尝试匹配include
                    if (regex_match(content.begin() + i, content.begin() + i + 7, includeRegex)) {
                        tokens.emplace_back(KEY_WORD, "include");
                        i = skip_blank(i + 7);
                        //成功匹配，开始检查后一个字符是否为<或者"
                        if (content[i] == '\"' || content[i] == '<') {
                            tokens.emplace_back(SEPARATOR, content[i]);
                            char right_flag = content[i] == '\"' ? '\"' : '>';
                            string lib;
                            i = skip_blank(i + 1);
                            while (content[i] != right_flag&&i<content.size()) {
                                lib += content[i++];
                            }
                            //缺少右部符号
                            if (i >= content.size()) {
                                cout << "error:lack of \" or > !" << endl;
                                exit(0);
                            }
                            tokens.emplace_back(IDENTIFIER, lib);
                            tokens.emplace_back(SEPARATOR, right_flag);
                            i = skip_blank(i + 1);
                            break;
                        }
                        //左部符号未匹配
                        else {
                            cout<< "error:lack of \" or < !" << endl;
                            exit(0);
                        }
                    }
                    //未找到include 头文件导入未匹配
                    else {
                        cout << "error:lack of include after # !" << endl;
                        exit(0);
                    }
                }
            }
            //识别标识符或者关键字
            else if (isalpha(content[i]) || content[i] == '_') {
                string temp;
                while (i < content.size() && (isalnum(content[i]) || content[i] == '_')) {
                    temp += content[i++];
                }
                if (is_keyword(temp)) {
                    tokens.emplace_back(KEY_WORD,temp);
                }
                else {
                    tokens.emplace_back(IDENTIFIER,temp);
                }
            }
            //如果是数字常量
            else if (isdigit(content[i])) {
                string temp;
                while (i < content.size()) {
                    if (isdigit(content[i]) || (i < content.size() - 1 && content[i] == '.' && isdigit(content[i + 1])))temp += content[i++];
                    else if (content[i] == '.') {
                        cout << "error:float type is lack of number after . !" << endl;
                        exit(0);
                    }
                    //单字节操作符或者双字节
                    else if (is_delimiters(content[i])||is_operators(string(1, content[i]))||is_operators(string(1, content[i]) + string(1, content[i+1]))) {
                        break;
                    }
                    //如果不是分隔符或者操作符结束则数字报错
                    else {
                        cout << "error:illegal char occurs in number denfinition!"<< endl;
                        exit(0);
                    }
                    i = skip_blank(i);
                }
                tokens.emplace_back(DIGIT_CONSTANT, temp);
            }
            //分隔符开头也可能是字符串常量
            else if(is_delimiters(content[i])) {
                tokens.emplace_back(SEPARATOR, content[i]);
                //如果是双引号开头 则是字符串常量
                if (content[i] == '\"') {
                    string temp ;
                    i++;
                    while (i < content.size()) {
                        if (content[i] != '\"') {
                            temp += content[i];
                            i++;
                        }
                        else break;
                    }
                    if (i >= content.size()) {
                        cout << "error:constant string is lack of \"" << endl;
                        exit(0);
                    }
                    tokens.emplace_back(STRING_CONSTANT, temp);
                    tokens.emplace_back(SEPARATOR, '\"');
                }
                i = skip_blank(i + 1);
            }
            //如果是运算符(单字节和双字节)
            else if (is_operators(string(1,content[i]))|| is_operators(string(1, content[i])+ string(1, content[i+1]))){
                //判断是自增或者自减符号
                if ((content[i] == '+' || content[i] == '-') && content[i+1] == content[i]) {
                    tokens.emplace_back(OPERATOR, string(2,content[i]));
                    i = skip_blank(i + 2);
                }
                //判断是大于等于或者小于等于或者不等于符号
                else if ((content[i] == '>' || content[i] == '<'||content[i]=='!'||content[i] == '=') && content[i + 1] == '=') {
                    tokens.emplace_back(OPERATOR, string(1,content[i])+string(1,'='));
                    i = skip_blank(i + 2);
                }
                //否则是单个运算符
                else {
                    tokens.emplace_back(OPERATOR, content[i]);
                    i = skip_blank(i + 1);
                }   
            }
            //如果是其他未支持的字符则跳过如  : ? . ||    ~  ^ <<  >>  |
            //支持识别但不支持功能            &&  +=  -=  *=  /=  %=  &=  |=  ^=  >>=  <<=
            else {
                i++;
                //待补充可支持的其他语法成分
                //1.三目运算符 示例 int a=b==1?1:0;
            }
        }
    }
    fs.close();
}