#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <iomanip>
#include <fstream>

using namespace std;

// 错误类型枚举
enum class ErrorType {
    LEXICAL_ERROR,      // 词法错误
    SYNTAX_ERROR,       // 语法错误
    SEMANTIC_ERROR,     // 语义错误
    WARNING            // 警告
};

// 错误严重程度
enum class ErrorSeverity {
    FATAL,      // 致命错误，停止编译
    ERROR,      // 错误，继续编译但不生成代码
    WARNING,    // 警告，继续编译
    INFO        // 信息，仅提示
};

// 错误恢复策略
enum class RecoveryStrategy {
    PANIC_MODE,         // 恐慌模式恢复
    PHRASE_LEVEL,       // 短语级恢复
    ERROR_PRODUCTION,   // 错误产生式恢复
    GLOBAL_CORRECTION   // 全局纠正
};

// 编译错误类
class CompilerError {
public:
    ErrorType type;
    ErrorSeverity severity;
    string message;
    string filename;
    int line;
    int column;
    string sourceCode;      // 出错的源代码行
    string suggestion;      // 修复建议
    RecoveryStrategy recovery;
    
    CompilerError(ErrorType t, ErrorSeverity s, const string& msg, 
                 const string& file = "", int l = 0, int c = 0,
                 const string& source = "", const string& suggest = "",
                 RecoveryStrategy rec = RecoveryStrategy::PANIC_MODE)
        : type(t), severity(s), message(msg), filename(file), 
          line(l), column(c), sourceCode(source), suggestion(suggest), recovery(rec) {}
    
    // 打印错误信息
    void print() const {
        // 错误类型和严重程度的颜色编码
        string typeColor = getTypeColor();
        string resetColor = "\033[0m";
        
        cout << typeColor;
        
        // 打印错误类型
        switch (type) {
            case ErrorType::LEXICAL_ERROR:
                cout << "词法错误";
                break;
            case ErrorType::SYNTAX_ERROR:
                cout << "语法错误";
                break;
            case ErrorType::SEMANTIC_ERROR:
                cout << "语义错误";
                break;
            case ErrorType::WARNING:
                cout << "警告";
                break;
        }
        
        // 打印位置信息
        if (!filename.empty() && line > 0) {
            cout << " 在 " << filename << ":" << line;
            if (column > 0) {
                cout << ":" << column;
            }
        }
        
        cout << resetColor << ": " << message << endl;
        
        // 打印源代码行（如果有）
        if (!sourceCode.empty()) {
            cout << "  " << setw(4) << line << " | " << sourceCode << endl;
            
            // 打印错误位置指示器
            if (column > 0) {
                cout << "       | ";
                for (int i = 1; i < column; i++) {
                    cout << " ";
                }
                cout << typeColor << "^" << resetColor << endl;
            }
        }
        
        // 打印修复建议
        if (!suggestion.empty()) {
            cout << "  建议: " << suggestion << endl;
        }
        
        cout << endl;
    }
    
private:
    string getTypeColor() const {
        switch (severity) {
            case ErrorSeverity::FATAL:
                return "\033[1;31m";  // 红色加粗
            case ErrorSeverity::ERROR:
                return "\033[31m";    // 红色
            case ErrorSeverity::WARNING:
                return "\033[33m";    // 黄色
            case ErrorSeverity::INFO:
                return "\033[36m";    // 青色
            default:
                return "";
        }
    }
};

// 错误处理器类
class ErrorHandler {
private:
    vector<CompilerError> errors;
    map<string, vector<string>> sourceLines;  // 文件名到源代码行的映射
    bool hasErrors;
    bool hasWarnings;
    int maxErrors;
    
public:
    ErrorHandler(int maxErr = 50) : hasErrors(false), hasWarnings(false), maxErrors(maxErr) {}
    
    // 加载源文件内容
    void loadSourceFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            reportError(ErrorType::LEXICAL_ERROR, ErrorSeverity::FATAL,
                       "无法打开源文件: " + filename, filename);
            return;
        }
        
        string line;
        vector<string> lines;
        while (getline(file, line)) {
            lines.push_back(line);
        }
        sourceLines[filename] = lines;
        file.close();
    }
    
    // 报告错误
    void reportError(ErrorType type, ErrorSeverity severity, const string& message,
                    const string& filename = "", int line = 0, int column = 0,
                    const string& suggestion = "", RecoveryStrategy recovery = RecoveryStrategy::PANIC_MODE) {
        
        // 获取源代码行
        string sourceCode = "";
        if (!filename.empty() && line > 0 && sourceLines.count(filename) > 0) {
            const auto& lines = sourceLines[filename];
            if (line <= (int)lines.size()) {
                sourceCode = lines[line - 1];
            }
        }
        
        CompilerError error(type, severity, message, filename, line, column, 
                           sourceCode, suggestion, recovery);
        errors.push_back(error);
        
        if (severity == ErrorSeverity::ERROR || severity == ErrorSeverity::FATAL) {
            hasErrors = true;
        } else if (severity == ErrorSeverity::WARNING) {
            hasWarnings = true;
        }
        
        // 如果错误数量超过限制，停止编译
        if (errors.size() >= maxErrors) {
            reportError(ErrorType::SYNTAX_ERROR, ErrorSeverity::FATAL,
                       "错误数量过多，停止编译", filename);
        }
    }
    
    // 报告词法错误
    void reportLexicalError(const string& message, const string& filename = "", 
                           int line = 0, int column = 0, const string& suggestion = "") {
        reportError(ErrorType::LEXICAL_ERROR, ErrorSeverity::ERROR, message, 
                   filename, line, column, suggestion);
    }
    
    // 报告语法错误
    void reportSyntaxError(const string& message, const string& filename = "", 
                          int line = 0, int column = 0, const string& suggestion = "") {
        reportError(ErrorType::SYNTAX_ERROR, ErrorSeverity::ERROR, message, 
                   filename, line, column, suggestion);
    }
    
    // 报告语义错误
    void reportSemanticError(const string& message, const string& filename = "", 
                            int line = 0, int column = 0, const string& suggestion = "") {
        reportError(ErrorType::SEMANTIC_ERROR, ErrorSeverity::ERROR, message, 
                   filename, line, column, suggestion);
    }
    
    // 报告警告
    void reportWarning(const string& message, const string& filename = "", 
                      int line = 0, int column = 0, const string& suggestion = "") {
        reportError(ErrorType::WARNING, ErrorSeverity::WARNING, message, 
                   filename, line, column, suggestion);
    }
    
    // 检测特定的不支持语法
    void checkUnsupportedSyntax(const string& text, const string& filename, int line) {
        // 检测 // 注释
        size_t pos = text.find("//");
        if (pos != string::npos) {
            int column = pos + 1;
            reportLexicalError("不支持的语法: 单行注释 '//'", filename, line, column,
                             "请使用当前语法支持的替代方案");
        }
        
        // 检测 /* */ 注释（如果也不支持）
        pos = text.find("/*");
        if (pos != string::npos) {
            int column = pos + 1;
            reportWarning("检测到不支持的语法, 多行注释", filename, line, column,
                         "请使用当前语法支持的替代方案");
        }
        
        // 检测其他不支持的语法
        vector<pair<string, string>> unsupportedPatterns = {
            {"++", "自增运算符 '++'"},
            {"--", "自减运算符 '--'"},
            {"+=", "复合赋值运算符 '+='"},
            {"-=", "复合赋值运算符 '-='"},
            {"*=", "复合赋值运算符 '*='"},
            {"/=", "复合赋值运算符 '/='"},
            {"&&", "逻辑与运算符 '&&'"},
            {"||", "逻辑或运算符 '||'"},
            {"!", "逻辑非运算符 '!'"},
            {"?", "三元运算符 '?:'"},
            {":", "三元运算符 '?:' 或标签"},
            {"#", "预处理指令"},
            {"include", "预处理指令 #include"},
            {"define", "预处理指令 #define"},
            {"for", "for循环语句"},
            {"do", "do-while循环语句"},
            {"switch", "switch语句"},
            {"case", "case语句"},
            {"break", "break语句"},
            {"continue", "continue语句"},
            {"struct", "结构体定义"},
            {"union", "联合体定义"},
            {"enum", "枚举定义"},
            {"typedef", "类型定义"},
            {"const", "常量修饰符"},
            {"static", "静态修饰符"},
            {"extern", "外部修饰符"}
        };
        
        for (const auto& pattern : unsupportedPatterns) {
            pos = text.find(pattern.first);
            if (pos != string::npos) {
                int column = pos + 1;
                reportLexicalError("不支持的语法: " + pattern.second, filename, line, column,
                                 "请使用当前语法支持的替代方案");
            }
        }
    }
    
    // 语法错误恢复
    bool attemptRecovery(RecoveryStrategy strategy, const string& context = "") {
        switch (strategy) {
            case RecoveryStrategy::PANIC_MODE:
                return panicModeRecovery(context);
            case RecoveryStrategy::PHRASE_LEVEL:
                return phraseLevelRecovery(context);
            case RecoveryStrategy::ERROR_PRODUCTION:
                return errorProductionRecovery(context);
            case RecoveryStrategy::GLOBAL_CORRECTION:
                return globalCorrectionRecovery(context);
            default:
                return false;
        }
    }
    
    // 打印所有错误
    void printAllErrors() const {
        if (errors.empty()) {
            cout << "\033[32m编译成功，无错误或警告。\033[0m" << endl;
            return;
        }
        
        cout << "\n=== 编译错误报告 ===" << endl;
        
        // 按类型分组统计
        int lexicalErrors = 0, syntaxErrors = 0, semanticErrors = 0, warnings = 0;
        
        for (const auto& error : errors) {
            error.print();
            
            switch (error.type) {
                case ErrorType::LEXICAL_ERROR:
                    lexicalErrors++;
                    break;
                case ErrorType::SYNTAX_ERROR:
                    syntaxErrors++;
                    break;
                case ErrorType::SEMANTIC_ERROR:
                    semanticErrors++;
                    break;
                case ErrorType::WARNING:
                    warnings++;
                    break;
            }
        }
        
        // 打印统计信息
        cout << "=== 错误统计 ===" << endl;
        cout << "词法错误: " << lexicalErrors << endl;
        cout << "语法错误: " << syntaxErrors << endl;
        cout << "语义错误: " << semanticErrors << endl;
        cout << "警告: " << warnings << endl;
        cout << "总计: " << errors.size() << " 个问题" << endl;
        
        if (hasErrors) {
            cout << "\033[31m编译失败。\033[0m" << endl;
        } else if (hasWarnings) {
            cout << "\033[33m编译完成，但有警告。\033[0m" << endl;
        }
    }
    
    // 获取错误统计
    bool hasCompilationErrors() const { return hasErrors; }
    bool hasCompilationWarnings() const { return hasWarnings; }
    size_t getErrorCount() const { return errors.size(); }
    
    // 清空错误列表
    void clear() {
        errors.clear();
        hasErrors = false;
        hasWarnings = false;
    }
    
private:
    // 恐慌模式恢复
    bool panicModeRecovery(const string& context) {
        // 跳过token直到遇到同步token（如分号、右括号等）
        cout << "尝试恐慌模式恢复..." << endl;
        return true;
    }
    
    // 短语级恢复
    bool phraseLevelRecovery(const string& context) {
        // 在当前位置插入、删除或替换token
        cout << "尝试短语级恢复..." << endl;
        return true;
    }
    
    // 错误产生式恢复
    bool errorProductionRecovery(const string& context) {
        // 使用特殊的错误产生式
        cout << "尝试错误产生式恢复..." << endl;
        return true;
    }
    
    // 全局纠正恢复
    bool globalCorrectionRecovery(const string& context) {
        // 全局分析和纠正
        cout << "尝试全局纠正恢复..." << endl;
        return true;
    }
};

// 全局错误处理器实例
extern ErrorHandler* globalErrorHandler;

// 便捷的错误报告函数
inline void reportLexicalError(const string& message, const string& filename = "", 
                              int line = 0, int column = 0, const string& suggestion = "") {
    if (globalErrorHandler) {
        globalErrorHandler->reportLexicalError(message, filename, line, column, suggestion);
    }
}

inline void reportSyntaxError(const string& message, const string& filename = "", 
                             int line = 0, int column = 0, const string& suggestion = "") {
    if (globalErrorHandler) {
        globalErrorHandler->reportSyntaxError(message, filename, line, column, suggestion);
    }
}

inline void reportSemanticError(const string& message, const string& filename = "", 
                               int line = 0, int column = 0, const string& suggestion = "") {
    if (globalErrorHandler) {
        globalErrorHandler->reportSemanticError(message, filename, line, column, suggestion);
    }
}

inline void reportWarning(const string& message, const string& filename = "", 
                         int line = 0, int column = 0, const string& suggestion = "") {
    if (globalErrorHandler) {
        globalErrorHandler->reportWarning(message, filename, line, column, suggestion);
    }
}

#ifdef ERROR_HANDLER_MAIN
// 主函数用于测试错误处理器
int main() {
    ErrorHandler *globalErrorHandler = nullptr;
    // 创建错误处理器实例
    ErrorHandler errorHandler;
    globalErrorHandler = &errorHandler;
    
    cout << "=== 错误处理器测试 ===" << endl;
    
    // 测试加载源文件
    string testFile = "./code/test_errors.src";
    cout << "正在测试错误处理器..." << endl;
    
    // 模拟一些错误
    globalErrorHandler->reportLexicalError("不支持的语法: 单行注释 '//'", 
                                         testFile, 1, 1, 
                                         "请使用 /* */ 多行注释格式，或删除注释");
    
    globalErrorHandler->reportSyntaxError("缺少分号", 
                                        testFile, 5, 15, 
                                        "在语句末尾添加分号");
    
    globalErrorHandler->reportSemanticError("变量未声明", 
                                          testFile, 8, 5, 
                                          "请先声明变量再使用");
    
    globalErrorHandler->reportWarning("变量未使用", 
                                    testFile, 3, 9, 
                                    "考虑删除未使用的变量");
    
    // 打印所有错误
    globalErrorHandler->printAllErrors();
    
    cout << "\n=== 错误统计 ===" << endl;
    cout << "是否有编译错误: " << (globalErrorHandler->hasCompilationErrors() ? "是" : "否") << endl;
    cout << "是否有警告: " << (globalErrorHandler->hasCompilationWarnings() ? "是" : "否") << endl;
    cout << "错误总数: " << globalErrorHandler->getErrorCount() << endl;
    
    return globalErrorHandler->hasCompilationErrors() ? 1 : 0;
} 
#endif // ERROR_HANDLER_H 