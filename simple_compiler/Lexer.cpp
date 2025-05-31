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
    //������
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
    // �򿪻򴴽��ļ�����д�롣
    std::ofstream outFile(split_filename+"_lexer.txt");
    // ����ļ��Ƿ�ɹ���
    if (!outFile.is_open()) {
        std::cerr << "�޷����ļ�����д�롣" << std::endl;
        return;
    }
    for (auto p : tokens) {
        // д�뵽�ļ���
        outFile << p.type_n << " " << p.value << std::endl;
    }
    // �ر��ļ�
    outFile.close();
}
void Lexer::main() {
    //�����ļ�
    std::ifstream fs(filename);
    if (!fs.is_open()) { 
        std::cout << "�޷����ļ�: " << filename << std::endl;
        return; 
    }
    is_open_file = true;
    while (getline(fs, content)) {
        size_t i = 0;
        while (i < content.size()) {
            i=skip_blank(i);
            //ʶ��ͷ�ļ�
            if (content[i] == '#') {
                tokens.emplace_back(SEPARATOR, content[i]);
                i=skip_blank(i+1);
                while (i < content.size()) {
                    regex includeRegex("include");
                    //����ƥ��include
                    if (regex_match(content.begin() + i, content.begin() + i + 7, includeRegex)) {
                        tokens.emplace_back(KEY_WORD, "include");
                        i = skip_blank(i + 7);
                        //�ɹ�ƥ�䣬��ʼ����һ���ַ��Ƿ�Ϊ<����"
                        if (content[i] == '\"' || content[i] == '<') {
                            tokens.emplace_back(SEPARATOR, content[i]);
                            char right_flag = content[i] == '\"' ? '\"' : '>';
                            string lib;
                            i = skip_blank(i + 1);
                            while (content[i] != right_flag&&i<content.size()) {
                                lib += content[i++];
                            }
                            //ȱ���Ҳ�����
                            if (i >= content.size()) {
                                cout << "error:lack of \" or > !" << endl;
                                exit(0);
                            }
                            tokens.emplace_back(IDENTIFIER, lib);
                            tokens.emplace_back(SEPARATOR, right_flag);
                            i = skip_blank(i + 1);
                            break;
                        }
                        //�󲿷���δƥ��
                        else {
                            cout<< "error:lack of \" or < !" << endl;
                            exit(0);
                        }
                    }
                    //δ�ҵ�include ͷ�ļ�����δƥ��
                    else {
                        cout << "error:lack of include after # !" << endl;
                        exit(0);
                    }
                }
            }
            //ʶ���ʶ�����߹ؼ���
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
            //��������ֳ���
            else if (isdigit(content[i])) {
                string temp;
                while (i < content.size()) {
                    if (isdigit(content[i]) || (i < content.size() - 1 && content[i] == '.' && isdigit(content[i + 1])))temp += content[i++];
                    else if (content[i] == '.') {
                        cout << "error:float type is lack of number after . !" << endl;
                        exit(0);
                    }
                    //���ֽڲ���������˫�ֽ�
                    else if (is_delimiters(content[i])||is_operators(string(1, content[i]))||is_operators(string(1, content[i]) + string(1, content[i+1]))) {
                        break;
                    }
                    //������Ƿָ������߲��������������ֱ���
                    else {
                        cout << "error:illegal char occurs in number denfinition!"<< endl;
                        exit(0);
                    }
                    i = skip_blank(i);
                }
                tokens.emplace_back(DIGIT_CONSTANT, temp);
            }
            //�ָ�����ͷҲ�������ַ�������
            else if(is_delimiters(content[i])) {
                tokens.emplace_back(SEPARATOR, content[i]);
                //�����˫���ſ�ͷ �����ַ�������
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
            //����������(���ֽں�˫�ֽ�)
            else if (is_operators(string(1,content[i]))|| is_operators(string(1, content[i])+ string(1, content[i+1]))){
                //�ж������������Լ�����
                if ((content[i] == '+' || content[i] == '-') && content[i+1] == content[i]) {
                    tokens.emplace_back(OPERATOR, string(2,content[i]));
                    i = skip_blank(i + 2);
                }
                //�ж��Ǵ��ڵ��ڻ���С�ڵ��ڻ��߲����ڷ���
                else if ((content[i] == '>' || content[i] == '<'||content[i]=='!'||content[i] == '=') && content[i + 1] == '=') {
                    tokens.emplace_back(OPERATOR, string(1,content[i])+string(1,'='));
                    i = skip_blank(i + 2);
                }
                //�����ǵ��������
                else {
                    tokens.emplace_back(OPERATOR, content[i]);
                    i = skip_blank(i + 1);
                }   
            }
            //���������δ֧�ֵ��ַ���������  : ? . ||    ~  ^ <<  >>  |
            //֧��ʶ�𵫲�֧�ֹ���            &&  +=  -=  *=  /=  %=  &=  |=  ^=  >>=  <<=
            else {
                i++;
                //�������֧�ֵ������﷨�ɷ�
                //1.��Ŀ����� ʾ�� int a=b==1?1:0;
            }
        }
    }
    fs.close();
}