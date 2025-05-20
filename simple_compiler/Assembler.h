#pragma once
#include<map>
#include<vector>
#include <string>
#include<fstream>>
#include "Parser.h"
#include <stack>
// 汇编文件处理器类
class AssemblerFileHandler {
private:
    std::vector<std::string> result;
    size_t dataPointer, bssPointer, textPointer;
public:
    AssemblerFileHandler() : dataPointer(1), bssPointer(3), textPointer(4) {
        result = { ".data", ".bss", ".lcomm bss_tmp, 4", ".text" };
    }

    size_t insert(const std::string& value, const std::string& type) {
        if (type == "DATA") {
            result.insert(result.begin() + dataPointer++, value);
            ++bssPointer;
            ++textPointer;
        }
        else if (type == "BSS") {
            result.insert(result.begin() + bssPointer++, value);
            ++textPointer;
        }
        else if (type == "TEXT") {
            result.insert(result.begin() + textPointer++, value);
            //返回在代码区的相对下标
            return textPointer-bssPointer - 1;
        }
        //报错
        return -1;
    }
    void change(const std::string& value,const int& index) {
        if (index < 0)return;
        result[index+bssPointer] = value;
    }
    void generateAssFile(const std::string& fileName) const {
        std::ofstream file(fileName + ".S");
        for (const auto& line : result) {
            file << line << "\n";
        }
        file.close();
    }
};

// 符号表条目结构,dataName表明存储的数据类型名称
class SymbolTableItem {
public:
    std::string type;
    std::string data_name;
    std::string data;
    SymbolTableItem() {};
    SymbolTableItem( const std::string&typei,const std::string& datanamei, const std::string& datai) :type(typei), data_name(datanamei), data(datai) {};
};
class OperandItem {
public:
    std::string type;
    std::string operand;
    std::string operand_array_index;
    OperandItem() {};
    OperandItem(const std::string& typei, const std::string& operandi) :type(typei), operand(operandi) {};
    void set_operand_array_index(const std::string& operand_array_indexi) {
        operand_array_index = operand_array_indexi;
    }
};
class ExpressionResult {
public:
    std::string type;
    std::string value;
    ExpressionResult() {};
    ExpressionResult(const std::string&typei,const std::string& valuei):type(typei),value(valuei){}
};
// Assembler 类
class Assembler {
private:
    SyntaxTree tree;
    AssemblerFileHandler ass_file_handler;
    //第一项为类型
    std::map<std::string, SymbolTableItem> symbol_table;
    //名字和返回值类型
    std::map<std::string, std::string>function_table;
    std:: vector<string> sentence_type = {"Program","Sentence","Include","FunctionStatement","Statement","FunctionCall","Assignment","Control","Expression","Return"};
    std::stack<string>operator_stack;
    std::stack<OperandItem>operand_stack;
    int jump_cnt = 0;
    int variable_cnt = 0;
    std::string filename;
public:
    Assembler(const std::string &filenamei):filename(filenamei) {
        Parser parser(filename);
        parser.main();
        parser.display();
        tree = parser.getTree();
    }
    void _include(std::shared_ptr<SyntaxTreeNode> node = NULL);
    void _function_statement(std::shared_ptr<SyntaxTreeNode> node = NULL);
    void _statement(std::shared_ptr<SyntaxTreeNode> node = NULL);
    void _function_call(std::shared_ptr<SyntaxTreeNode> node = NULL);
    void _assignment(std::shared_ptr<SyntaxTreeNode> node = NULL);
    void _control_for(std::shared_ptr<SyntaxTreeNode> node = NULL);
    void _control_if(std::shared_ptr<SyntaxTreeNode> node = NULL);
    void _control_while(std::shared_ptr<SyntaxTreeNode> node = NULL);
    void _return(std::shared_ptr<SyntaxTreeNode> node = NULL);
    void _traverse_expression(std::shared_ptr<SyntaxTreeNode> node = NULL);
    ExpressionResult _expression(std::shared_ptr<SyntaxTreeNode> node = NULL);
    void _handler_block(std::shared_ptr<SyntaxTreeNode> node = NULL);
    //遍历语法树生成汇编代码
    void traverse(std::shared_ptr<SyntaxTreeNode> node = NULL);
    //根据变量类型返回数据长度
    string _sizeof(const std::string& _type);
    //判断是否是float类型
    bool _is_float(OperandItem& operand);
    //判断是否有float类型的数
    bool _contain_float(OperandItem& operand_a, OperandItem& operand_b);
    //判断是否有对应sentence
    bool _is_contain_sentence(const std::string& sentence);
    bool _is_contain_function(const std::string& function_name);
    string join(const std::vector<std::string>& list, const std::string& sep);
    void main();
};

