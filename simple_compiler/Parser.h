#pragma once
#include"Lexer.h"
#include"SyntaxTree.h"
class Parser {
private:
    std::vector<Token> tokens;
    SyntaxTree tree;
    //当前遍历的token下标
    int index = 0;
public:
    Parser(const std::string& filename){
        //调试用
        //Lexer lexer(filename);
        //lexer.main();
        //lexer.display();
        //tokens = lexer.getTokens();
        readTokensFromFile(filename);
    }
    void _block(SyntaxTree fathertree);
    void _include(std::shared_ptr<SyntaxTreeNode> father=NULL);
    void _function_statement(std::shared_ptr<SyntaxTreeNode> father = NULL);
    void _statement(std::shared_ptr<SyntaxTreeNode> father = NULL);
    void _assignment(std::shared_ptr<SyntaxTreeNode> father = NULL);
    void _while(std::shared_ptr<SyntaxTreeNode> father = NULL);
    void _for(std::shared_ptr<SyntaxTreeNode> father = NULL);
    void _if_else(std::shared_ptr<SyntaxTreeNode> father = NULL);
    void _control(std::shared_ptr<SyntaxTreeNode> father = NULL);
    void _expression(std::shared_ptr<SyntaxTreeNode> father = NULL, int in_index = 0);
    void _function_call(std::shared_ptr<SyntaxTreeNode> father = NULL);
    void _return(std::shared_ptr<SyntaxTreeNode> father = NULL);
    string _judge_sentence_pattern();
    void main();
    void dfs(std::shared_ptr<SyntaxTreeNode> node);
    void display();
    void readTokensFromFile(const std::string& filename);
    SyntaxTree getTree() { return tree; };
};
