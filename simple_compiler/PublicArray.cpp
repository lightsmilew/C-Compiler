#include<map>
#include<vector>
#include <string>
using namespace std;
map<string, string>map_token_type = {
    //1,2,5与TokenType值对应
    {"1","IDENTIFIER"},
    {"2","DIGIT_CONSTANT"},
    {"5","STRING_CONSTANT"},
    {"include","INCLUDE"},
    {"int", "INT"},
    {"float", "FLOAT"},
    {"char", "CHAR"},
    {"double", "DOUBLE"},
    {"for", "FOR"},
    {"if", "IF"},
    {"else", "ELSE"},
    {"while", "WHILE"},
    {"do", "DO"},
    {"return", "RETURN"},
    {"=", "ASSIGN"},
    {"&", "ADDRESS"},
    {"<", "LT"},
    {">", "GT"},
    {"++", "SELF_PLUS"},
    {"--", "SELF_MINUS"},
    {"+", "PLUS"},
    {"-", "MINUS"},
    {"*", "MUL"},
    {"/", "DIV"},
    {">=", "GET"},
    {"<=", "LET"},
    {"==", "EQUAL"},
    {"!=", "NOT_EQUAL"},
    {"(", "LL_BRACKET"},
    {")", "RL_BRACKET"},
    {"{", "LB_BRACKET"},
    {"}", "RB_BRACKET"},
    {"[", "LM_BRACKET"},
    {"]", "RM_BRACKET"},
    {",", "COMMA"},
    {"\"", "DOUBLE_QUOTE"},
    {";", "SEMICOLON"},
    {"#", "SHARP"}
};
// 关键字
vector<vector<string>> keywords = {
    {"int", "float", "double", "char", "void"},
    {"if", "for", "while", "do", "else"},
    {"include", "return"}
};

// 运算符
vector<string> operators = {
    "=", "&", "<", ">", "++", "--", "+", "-", "*", "/", ">=", "<=","==", "!=","!"
};

// 分隔符
vector<char> delimiters = {
    '(', ')', '{', '}', '[', ']', ',', '\"', ';'
};
//单目运算符
vector<string> single_operators = { "!", "++", "--" };
//双目运算符
vector<string> double_operators = { "+", "-", "*", "/", ">", "<", ">=", "<=","==","!=" };
// 运算符对汇编指令的映射
map<string, string> operator_map = { {">", "jbe"}, {"<", "jae"}, {">=", "jb"}, {"<=", "ja"} ,{"==","jne"},{"!=","je"} };
