#pragma once
#include<vector>
#include"Token.h"
class Lexer {
private:
    string content;
    string filename;
    vector<Token> tokens;

    size_t skip_blank(size_t i);
    bool is_blank(char c);
    bool is_keyword(const std::string& value);
    bool is_delimiters(char c);
    bool is_operators(string str);
public:
    Lexer(const string& filename) : filename(filename) {}
    vector<Token>& getTokens() { return tokens; }
    void main();
    void display();
};
