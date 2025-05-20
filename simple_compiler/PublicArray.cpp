#include<map>
#include<vector>
#include <string>
using namespace std;
map<string, string>map_token_type = {
    //1,2,5��TokenTypeֵ��Ӧ
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
// �ؼ���
vector<vector<string>> keywords = {
    {"int", "float", "double", "char", "void"},
    {"if", "for", "while", "do", "else"},
    {"include", "return"}
};

// �����
vector<string> operators = {
    "=", "&", "<", ">", "++", "--", "+", "-", "*", "/", ">=", "<=","==", "!=","!"
};

// �ָ���
vector<char> delimiters = {
    '(', ')', '{', '}', '[', ']', ',', '\"', ';'
};
//��Ŀ�����
vector<string> single_operators = { "!", "++", "--" };
//˫Ŀ�����
vector<string> double_operators = { "+", "-", "*", "/", ">", "<", ">=", "<=","==","!=" };
// ������Ի��ָ���ӳ��
map<string, string> operator_map = { {">", "jbe"}, {"<", "jae"}, {">=", "jb"}, {"<=", "ja"} ,{"==","jne"},{"!=","je"} };
