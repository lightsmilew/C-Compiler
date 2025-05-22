#pragma once
#include <string>
#include <vector>
#include <map>
using namespace std;
enum TokenType {
    KEY_WORD, IDENTIFIER, DIGIT_CONSTANT, OPERATOR, SEPARATOR, STRING_CONSTANT
};
extern map<string,string> map_token_type;
struct Token {
    TokenType type;
    string type_n;
    string value;

    Token(TokenType t, const std::string& v):type(t),value(v){
        type_n = (type == IDENTIFIER || type == DIGIT_CONSTANT || type == STRING_CONSTANT) ? map_token_type[to_string(type)] : map_token_type[value];
    }
    Token(TokenType t, char v) :type(t) {
        //ÀàÐÍ×ª»»
        value = string(1, v);
        type_n = (type == IDENTIFIER || type == DIGIT_CONSTANT || type == STRING_CONSTANT) ? map_token_type[to_string(type)] : map_token_type[value];
    }
    Token(string type_n_in, string v_in) :type_n(type_n_in), value(v_in) {};
};