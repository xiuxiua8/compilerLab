#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <memory>
#include <regex>

using namespace std;

// Token结构
struct Token {
    string type;
    string value;
    int line;
    
    Token(const string& t, const string& v, int l = 0) 
        : type(t), value(v), line(l) {}
};

// 编译器驱动类
class CompilerDriver {
private:
    string sourceFile;
    vector<Token> tokens;
    
public:
    CompilerDriver(const string& file) : sourceFile(file) {}
    
    // 第一步：词法分析
    bool runLexicalAnalysis() {
        cout << "=== 词法分析 ===" << endl;
        
        // 调用lab1的词法分析器
        string absPath = "../" + sourceFile;  // 需要相对于lab1目录的路径
        string command = "cd lab1 && echo -e '3\\n" + absPath + "' | ./dfa 2>&1";
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            cerr << "无法运行词法分析器" << endl;
            return false;
        }
        
        char buffer[256];
        string result;
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        int ret = pclose(pipe);
        
        if (ret != 0) {
            cerr << "词法分析器执行失败" << endl;
            return false;
        }
        
        // 解析词法分析输出
        parseLexerOutput(result);
        
        // 添加结束符
        tokens.push_back(Token("#", "#", tokens.empty() ? 1 : tokens.back().line));
        
        cout << "词法分析完成，共识别 " << tokens.size() - 1 << " 个token" << endl;
        return true;
    }
    
    // 解析词法分析器输出
    void parseLexerOutput(const string& output) {
        istringstream iss(output);
        string line;
        regex tokenRegex(R"(\((\w+),\s*([^)]+)\))");
        
        int lineNum = 0;
        while (getline(iss, line)) {
            // 查找行号标记
            if (line.find("第") != string::npos && line.find("行:") != string::npos) {
                size_t start = line.find("第") + 3;
                size_t end = line.find("行:");
                if (start < end) {
                    lineNum = stoi(line.substr(start, end - start));
                }
                
                // 解析该行的tokens
                smatch match;
                string::const_iterator searchStart(line.cbegin());
                while (regex_search(searchStart, line.cend(), match, tokenRegex)) {
                    string type = match[1];
                    string value = match[2];
                    
                    // 处理特殊情况
                    if (type == "INT_NUM") type = "NUM";
                    if (type == "FLOAT_NUM") type = "FLO";
                    
                    tokens.push_back(Token(type, value, lineNum));
                    searchStart = match.suffix().first;
                }
            }
        }
    }
    
    // 第二步：语法分析
    bool runSyntaxAnalysis() {
        cout << "\n=== 语法分析 ===" << endl;
        
        // 将tokens写入临时文件
        ofstream tmpFile("tokens.tmp");
        for (const auto& token : tokens) {
            tmpFile << token.type << " " << token.value << endl;
        }
        tmpFile.close();
        
        // 调用lab3的语法分析器（如果有的话）
        // 这里我们暂时只打印tokens
        cout << "Token序列：" << endl;
        for (const auto& token : tokens) {
            cout << "(" << token.type << ", " << token.value << ") ";
        }
        cout << endl;
        
        // TODO: 集成SLR(1)语法分析
        
        return true;
    }
    
    // 第三步：语义分析和AST构建
    bool runSemanticAnalysis() {
        cout << "\n=== 语义分析 ===" << endl;
        
        // TODO: 基于语法分析结果构建AST
        // TODO: 进行类型检查等语义分析
        
        cout << "语义分析需要先完成语法分析的集成" << endl;
        
        return true;
    }
    
    // 第四步：中间代码生成
    bool generateIntermediateCode() {
        cout << "\n=== 中间代码生成 ===" << endl;
        
        // TODO: 基于AST生成四元式
        
        cout << "中间代码生成需要先完成AST构建" << endl;
        
        return true;
    }
    
    // 运行完整的编译流程
    bool compile() {
        if (!runLexicalAnalysis()) {
            return false;
        }
        
        if (!runSyntaxAnalysis()) {
            return false;
        }
        
        if (!runSemanticAnalysis()) {
            return false;
        }
        
        if (!generateIntermediateCode()) {
            return false;
        }
        
        cout << "\n编译完成！" << endl;
        return true;
    }
    
    // 保存tokens到文件
    void saveTokensToFile(const string& filename) {
        ofstream out(filename);
        for (const auto& token : tokens) {
            out << token.line << " " << token.type << " " << token.value << endl;
        }
        out.close();
        cout << "Tokens已保存到 " << filename << endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "用法: " << argv[0] << " <源文件>" << endl;
        cerr << "示例: " << argv[0] << " code/10.src" << endl;
        return 1;
    }
    
    string sourceFile = argv[1];
    
    // 检查文件是否存在
    ifstream test(sourceFile);
    if (!test.good()) {
        cerr << "错误：无法打开文件 " << sourceFile << endl;
        return 1;
    }
    test.close();
    
    // 创建编译器驱动
    CompilerDriver driver(sourceFile);
    
    // 运行编译
    if (!driver.compile()) {
        cerr << "编译失败" << endl;
        return 1;
    }
    
    // 保存tokens供后续使用
    driver.saveTokensToFile("tokens.txt");
    
    return 0;
} 