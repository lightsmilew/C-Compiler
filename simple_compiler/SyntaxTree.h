#pragma once
#include <string>
#include<memory>
// AST节点类
class SyntaxTreeNode {
public:
    std::string node_value, node_type;
    std::map<std::string,std::string>extra_info;
    //左右兄弟节点、父亲节点、第一个孩子节点
    //智能指针不需要显式初始化为NULL
    std::shared_ptr<SyntaxTreeNode> left, right,father,first_son;
    SyntaxTreeNode(const std::string& value="", const std::string& type="", const std::string& info_1="") :node_value(value), node_type(type) {
       if(info_1!="")extra_info["type"] = info_1;
    }
    void set_type(const std::string& type) {
        node_type = type;
    }
    void set_value(const std::string& value) {
        node_value = value;
    }
    void set_extra_info(const std::string& datatype, const::string& data) {
        extra_info[datatype] = data;
    }
};
class SyntaxTree {
public :
    std::shared_ptr<SyntaxTreeNode>root, current;
    SyntaxTree(){}
    //添加孩子节点
    void add_child_node(std::shared_ptr<SyntaxTreeNode>new_node, std::shared_ptr<SyntaxTreeNode>father = NULL) {
        //传入父节点为空，则从当前节点开始遍历
        if (father == NULL)father = current;
        new_node->father = father;
        //如果父节点第一个孩子节点为空则直接添加，否则遍历到最后一个孩子节点再添加
        if (father->first_son == NULL)father->first_son = new_node;
        else {
            std::shared_ptr<SyntaxTreeNode>current_node = father->first_son;
            while (current_node->right != NULL)current_node = current_node->right;
            current_node->right = new_node;
            new_node->left = current_node;
        }
        current = new_node;
    }
    //交换左右子树
    void exchange_lr_tree(std::shared_ptr<SyntaxTreeNode>left, std::shared_ptr<SyntaxTreeNode>right) {
        std::shared_ptr<SyntaxTreeNode>left_left = left->left;
        std::shared_ptr<SyntaxTreeNode>right_right = right->right;
        left->left = right;
        left->right = right_right;
        right->left = left_left;
        right->right = left;
        if (left_left != NULL)left_left->right = right;
        if (right_right != NULL)right_right->left = left;
    }
};
