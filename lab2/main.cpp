#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <unordered_set>
#include <algorithm>
#include <regex>

// 定义关键字集合
std::unordered_set<std::string> keywords = {
    "if", "else", "while", "for", "int", "float", "double", "return", "void", "break", "continue", "input", "print"
};

// 判断符号类型的函数
std::string getTokenType(const std::string& token) {
    // 如果是空字符串，返回空
    if (token.empty()) {
        return "";
    }
    
    // 检查是否是关键字
    if (keywords.find(token) != keywords.end()) {
        std::string upperKeyword = token;
        std::transform(upperKeyword.begin(), upperKeyword.end(), upperKeyword.begin(), ::toupper);
        return upperKeyword;
    }
    
    // 检查是否是标识符
    if (std::regex_match(token, std::regex("[a-zA-Z_][a-zA-Z0-9_]*"))) {
        return "ID";
    }
    
    // 检查是否是整数
    if (std::regex_match(token, std::regex("[+-]?[0-9]+"))) {
        return "NUM";
    }
    
    // 检查是否是浮点数
    if (std::regex_match(token, std::regex("[+-]?[0-9]*\\.[0-9]+"))) {
        return "FLOAT";
    }
    
    // 检查运算符
    if (token == "+") return "ADD"; // 加号 add
    if (token == "-") return "SUB"; // 减号 sub
    if (token == "/") return "DIV"; // 除号 div
    if (token == "*") return "MUL"; // 乘号 mul
    if (token == "<" || token == "<=" || token == "==") return "ROP"; // 关系运算符 relop
    if (token == "=") return "ASG"; // 赋值运算符 assign
    if (token == "(") return "LPA"; // 左括号 left parenthesis
    if (token == ")") return "RPA"; // 右括号 right parenthesis
    if (token == "[") return "LBK"; // 左方括号 left bracket    
    if (token == "]") return "RBK"; // 右方括号 right bracket
    if (token == "{") return "LBR"; // 左大括号 left brace
    if (token == "}") return "RBR"; // 右大括号 right brace
    if (token == ",") return "CMA"; // 逗号 comma
    if (token == ";") return "SCO"; // 分号 semicolon   
    
    // 其他情况，返回ID
    return "ID";
}

// 词法分析函数
std::vector<std::pair<std::string, std::string>> lexicalAnalysis(const std::string& line) {
    std::vector<std::pair<std::string, std::string>> tokens;
    std::string currentToken;
    
    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        
        // 跳过空白字符
        if (isspace(c)) {
            if (!currentToken.empty()) {
                std::string type = getTokenType(currentToken);
                tokens.push_back({currentToken, type});
                currentToken.clear();
            }
            continue;
        }
        
        // 处理分隔符和运算符
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || 
            c == '(' || c == ')' || c == '{' || c == '}' || c == ';' || 
            c == ',' || c == '<' || c == '>' || c == '!' || c == '[' || c == ']') {
            
            // 先保存当前的token
            if (!currentToken.empty()) {
                std::string type = getTokenType(currentToken);
                tokens.push_back({currentToken, type});
                currentToken.clear();
            }
            
            // 检查是否是双字符运算符
            if (i + 1 < line.size()) {
                char nextChar = line[i + 1];
                if ((c == '=' && nextChar == '=') || 
                    (c == '!' && nextChar == '=') ||
                    (c == '<' && nextChar == '=') ||
                    (c == '>' && nextChar == '=')) {
                    
                    std::string op;
                    op += c;
                    op += nextChar;
                    tokens.push_back({op, getTokenType(op)});
                    i++;  // 跳过下一个字符
                    continue;
                }
            }
            
            // 单字符运算符
            std::string op(1, c);
            tokens.push_back({op, getTokenType(op)});
            continue;
        }
        
        // 添加到当前token
        currentToken += c;
    }
    
    // 处理最后一个token
    if (!currentToken.empty()) {
        std::string type = getTokenType(currentToken);
        tokens.push_back({currentToken, type});
    }
    
    return tokens;
}

int main() {
    int mode;
    std::cout << "请选择运行模式 (1: 分析单个符号, 2: 分析整行语句): ";
    std::cin >> mode;
    
    if (mode == 1) {
        // 模式1：分析单个符号
        int n;
        std::cin >> n;
        std::cout << "请输入" << n << "个用空格分隔的符号串: ";
        for (int i = 0; i < n; i++) {
            std::string token;
            std::cin >> token;
            std::string type = getTokenType(token);
            std::cout << "(" << type << ", " << token << ") ";
            std::cout << std::endl;
        }
        //std::cout << std::endl;
    } else if (mode == 2) {
        // 模式2：分析整行语句
        std::cin.ignore();  // 清除输入缓冲区
        std::cout << "请输入一行语句进行词法分析: ";
        std::string line;
        std::getline(std::cin, line);

        auto tokens = lexicalAnalysis(line
        for (const auto& token : tokens) {
            std::cout << "("  << token.second << ", "  << token.first << ") ";
            std::cout << std::endl;
        }
        //std::cout << std::endl;
    } else {
        std::cout << "无效的模式选择！" << std::endl;
    }
    
    return 0;
}
