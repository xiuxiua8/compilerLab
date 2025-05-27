#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <memory>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <stack>
#include <queue>
#include "lab1/dfa.cpp"

using namespace std;

// ===== 词法分析器（简化版）=====
enum class TokenType {
    // 关键字
    INT, FLOAT, VOID, IF, ELSE, WHILE, RETURN,
    // 标识符和字面量
    ID, INT_NUM, FLOAT_NUM,
    // 运算符
    ADD, MUL, ASG, REL_OP,
    // 分隔符
    SEMI, COMMA, LPAR, RPAR, LBR, RBR, LBRACK, RBRACK,
    // 特殊
    EOF_TOKEN, UNKNOWN
};

string tokenTypeToString(TokenType type) { 
    switch (type) {
        case TokenType::INT: return "INT";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::VOID: return "VOID";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::WHILE: return "WHILE";
        case TokenType::RETURN: return "RETURN";
        case TokenType::ID: return "ID";
        case TokenType::INT_NUM: return "INT_NUM";
        case TokenType::FLOAT_NUM: return "FLOAT_NUM";
        case TokenType::ADD: return "ADD";
        case TokenType::MUL: return "MUL";
        case TokenType::ASG: return "ASG";
        case TokenType::REL_OP: return "REL_OP";
        case TokenType::SEMI: return "SEMI";
        case TokenType::COMMA: return "COMMA";
        case TokenType::LPAR: return "LPAR";
        case TokenType::RPAR: return "RPAR";
        case TokenType::LBR: return "LBR";
        case TokenType::RBR: return "RBR";
        case TokenType::LBRACK: return "LBRACK";
        case TokenType::RBRACK: return "RBRACK";
        case TokenType::EOF_TOKEN: return "EOF_TOKEN";
        case TokenType::UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN";
    }
} 


struct Token {
    TokenType type;
    string value;
    string lexeme;
    int line;
    int column;
};

class Lexer {
private:

    string filename;
    size_t pos;
    int lineNumber;
    int column;
    vector<Token> tokens;
    
    map<string, TokenType> keywords = {
        {"int", TokenType::INT},
        {"float", TokenType::FLOAT},
        {"void", TokenType::VOID},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"while", TokenType::WHILE},
        {"return", TokenType::RETURN}
    };
    
public:
    Lexer(const string filename) : filename(filename), pos(0), lineNumber(1), column(1) {
        DFA dfa;
        // 初始化关键字表
        dfa.initKeywords();

        if (!dfa.loadFromFile("./lab1/dfa.txt")) {
            cout << "无法打开 DFA 配置文件。\n";
        }

        if (!dfa.validate()) {
            cout << "DFA 验证失败。\n";
        } 
        
        // 读取文件内容
        ifstream file(filename);
        if (!file) {
            cout << "无法打开文件: " << filename << endl;
        }
        
        string line;
        vector<pair<string, string>> allResults; // 存储所有行的分析结果
        
        cout << "开始分析文件: " << filename << endl;
        
        // 逐行读取并分析文件
        while (getline(file, line)) {
            vector<string> tokensvalue = dfa.tokenizeInput(line);
            vector<pair<string, string>> results; // 当前行的结果

            for (const auto& tokenvalue : tokensvalue) {
                string endState = dfa.getEndState(tokenvalue);
                if (endState != "ERROR" && dfa.getAcceptStates().count(endState) > 0) {
                    string type = dfa.getStateType(endState);
                    type = dfa.classifyToken(type, tokenvalue);
                    Token token;
                    token.type = classifyToken(type);
                    token.value = tokenvalue;
                    token.line = lineNumber;
                    token.column = 1;
                    tokens.push_back(token);
                } else {
                    Token token;
                    token.type = classifyToken("ERROR");
                    token.value = tokenvalue;
                    token.line = lineNumber;
                    token.column = 1;
                    tokens.push_back(token);
                }
            }
            lineNumber++;
        }

        file.close();

    }
    
    Token getNextToken() {
        if (pos < tokens.size()) {
            return tokens[pos++];
        }
        return Token(); 
    }

    int getPos() {
        return pos;
    }

    int getTokensSize() {
        return tokens.size();
    }

    void printTokens() {
        for (const auto& token : tokens) {
            cout << " (" << tokenTypeToString(token.type) << ", " << token.value << ") " << endl;
        }
        cout << endl;
    }
    
private:
    TokenType classifyToken(const string& type) {
        //DIV MUL ASG LPA RPA LBK RBK LBR RBR  CMA SCO ROP  ID ADD IF ELSE WHILE RETURN INT FLOAT VOID
        //INT, FLOAT, VOID, IF, ELSE, WHILE, RETURN, ID, INT_NUM, FLOAT_NUM, ADD, MUL, ASG, REL_OP, SEMI, COMMA, LPAR, RPAR, LBR, RBR, LBRACK, RBRACK, EOF_TOKEN, UNKNOWN
        if (type == "INT") {
            return TokenType::INT;
        } else if (type == "FLOAT") {
            return TokenType::FLOAT;
        } else if (type == "VOID") {
            return TokenType::VOID;
        } else if (type == "IF") {
            return TokenType::IF;
        } else if (type == "ELSE") {
            return TokenType::ELSE;
        } else if (type == "WHILE") {
            return TokenType::WHILE;
        } else if (type == "RETURN") {
            return TokenType::RETURN;
        } else if (type == "ID") {
            return TokenType::ID;
        } else if (type == "INT_NUM") {
            return TokenType::INT_NUM;
        } else if (type == "FLO") {
            return TokenType::FLOAT_NUM;
        } else if (type == "ADD") {
            return TokenType::ADD;
        } else if (type == "MUL") {
            return TokenType::MUL;
        } else if (type == "ASG") {
            return TokenType::ASG;
        } else if (type == "ROP") {
            return TokenType::REL_OP;
        } else if (type == "SCO") {
            return TokenType::SEMI;
        } else if (type == "CMA") {
            return TokenType::COMMA;
        }  else if (type == "LBR") {
            return TokenType::LBR;
        } else if (type == "RBR") {
            return TokenType::RBR;
        } else if (type == "LBK") {
            return TokenType::LBRACK;
        } else if (type == "RBK") {
            return TokenType::RBRACK;
        } else if (type == "LPA") {
            return TokenType::LPAR;
        } else if (type == "RPA") {
            return TokenType::RPAR;
        } else if (type == "UNKNOWN") {
            return TokenType::UNKNOWN;
        } else {
            return TokenType::UNKNOWN;
        }
    }

};


int lexer_test() {
    Lexer lexer("./code/19.src");
    lexer.printTokens();
    return 0;
}