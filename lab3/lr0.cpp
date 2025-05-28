#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <sstream>
#include <algorithm>
#include <queue>
#include <iomanip>
#include <tuple>

using namespace std;

// 调试标志，设置为false关闭所有调试输出
bool DEBUG_MODE = false;

// 调试输出封装
#define DEBUG_PRINT(x) if(DEBUG_MODE) { x; }

// 文法产生式
struct Production {
    string left;
    vector<string> right;
};


// 文法
class Grammar {
public:
    vector<Production> productions;
    set<string> nonterminals;
    set<string> terminals;
    string start_symbol;
    map<string, set<string>> first;
    map<string, set<string>> follow;

    void parse(const vector<string>& rules);
    void compute_first();
    void compute_follow();
    void print_grammar();
    bool compute_first_of_string(const vector<string>& str, set<string>& result);
};

void Grammar::parse(const vector<string>& rules) {
    productions.clear();
    nonterminals.clear();
    terminals.clear();
    for (const auto& rule : rules) {
        size_t arrow = rule.find("→");
        if (arrow == string::npos) arrow = rule.find("->");
        if (arrow == string::npos) continue;
        string left = rule.substr(0, arrow);
        left.erase(remove_if(left.begin(), left.end(), ::isspace), left.end());
        nonterminals.insert(left);
        string right_all = rule.substr(arrow + (rule[arrow] == '-' ? 2 : 3));
        stringstream ss(right_all);
        string prod;
        while (getline(ss, prod, '|')) {
            vector<string> symbols;
            stringstream ssp(prod);
            string sym;
            while (ssp >> sym) {
                if (sym != "ε") {  // 跳过ε符号，让空产生式的right为空数组
                    symbols.push_back(sym);
                }
            }
            productions.push_back({left, symbols});
        }
    }
    // 统计终结符
    for (const auto& prod : productions) {
        for (const auto& sym : prod.right) {
            if (nonterminals.count(sym) == 0 && sym != "" && sym != "ε") {
                terminals.insert(sym);
            }
        }
    }
    if (!productions.empty()) {
        start_symbol = productions[0].left;
    }
    
    //augment
    //string new_start = start_symbol + "'";
    string new_start = "S'";
    productions.insert(productions.begin(), {new_start, {start_symbol}});
    nonterminals.insert(new_start);
    start_symbol = new_start;
}
void Grammar::compute_first() {
    // 初始化：终结符的FIRST集就是它自身
    for (const auto& t : terminals) first[t] = {t};
    // 初始化：空串的FIRST集就是空串自身
    first["ε"] = {"ε"};
    // 初始化：所有非终结符的FIRST集为空集
    for (const auto& nt : nonterminals) first[nt] = {};
    
    // 使用固定点算法计算FIRST集，直到没有任何变化
    bool changed = true;
    while (changed) {
        changed = false;
        // 遍历所有产生式
        for (const auto& prod : productions) {
            string A = prod.left;
            const vector<string>& alpha = prod.right;
            
            // 处理空产生式: A → ε
            if (alpha.empty() || (alpha.size() == 1 && alpha[0] == "ε")) {
                if (first[A].insert("ε").second) {
                    changed = true;
                }
                continue;
            }
            
            // 遍历产生式右部的每个符号
            bool all_nullable = true;
            for (size_t i = 0; i < alpha.size(); ++i) {
                string symbol = alpha[i];
                bool symbol_nullable = false;
                
                // 将FIRST(symbol)中除ε外的所有符号加入FIRST(A)
                for (const auto& f : first[symbol]) {
                    if (f == "ε") {
                        symbol_nullable = true;
                    } else if (first[A].insert(f).second) {
                        changed = true;
                    }
                }
                
                // 如果当前符号不可空，则后续符号不会对FIRST(A)有贡献
                if (!symbol_nullable) {
                    all_nullable = false;
                    break;
                }
            }
            
            // 如果产生式右部所有符号都可空，则将ε加入FIRST(A)
            if (all_nullable && first[A].insert("ε").second) {
                changed = true;
            }
        }
    }
    
    // 输出FIRST集，方便调试
    if (DEBUG_MODE) {
        cout << "=== FIRST集 ===" << endl;
        for (const auto& nt : nonterminals) {
            cout << "FIRST(" << nt << ") = { ";
            for (const auto& f : first[nt]) cout << f << ' ';
            cout << "}" << endl;
        }
        
        // 也输出终结符的FIRST集
        for (const auto& t : terminals) {
            cout << "FIRST(" << t << ") = { ";
            for (const auto& f : first[t]) cout << f << ' ';
            cout << "}" << endl;
        }
    }
}

void Grammar::compute_follow() {
    follow.clear();
    // 初始化：所有非终结符的FOLLOW集为空集
    for (const auto& nt : nonterminals) follow[nt] = {};
    // 规则1：将输入结束符#加入FOLLOW(S)，其中S是文法的开始符号
    follow[start_symbol].insert("#");
    
    bool changed = true;
    // 使用固定点算法计算FOLLOW集，直到没有任何变化
    while (changed) {
        changed = false;
        // 遍历所有产生式
        for (const auto& prod : productions) {
            const string& A = prod.left;
            
            // 遍历产生式右部的每个符号
            for (size_t i = 0; i < prod.right.size(); ++i) {
                const string& B = prod.right[i];
                // 只处理非终结符
                if (nonterminals.count(B)) {
                    // 规则2：对于产生式A→αBβ，将FIRST(β)中除ε外的所有符号加入FOLLOW(B)
                    if (i + 1 < prod.right.size()) {
                        // 构建β
                        vector<string> beta(prod.right.begin() + i + 1, prod.right.end());
                        
                        // 计算FIRST(β)
                        set<string> first_beta;
                        bool beta_contains_epsilon = compute_first_of_string(beta, first_beta);
                        
                        // 将FIRST(β)中除ε外的所有符号加入FOLLOW(B)
                        for (const auto& f : first_beta) {
                            if (f != "ε" && follow[B].insert(f).second) {
                                changed = true;
                            }
                        }
                        
                        // 规则3：如果β可导出ε，将FOLLOW(A)加入FOLLOW(B)
                        if (beta_contains_epsilon) {
                            for (const auto& f : follow[A]) {
                                if (follow[B].insert(f).second) {
                                    changed = true;
                                }
                            }
                        }
                    } 
                    // 规则3：对于产生式A→αB，将FOLLOW(A)加入FOLLOW(B)
                    else {
                        for (const auto& f : follow[A]) {
                            if (follow[B].insert(f).second) {
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }
    
    if (DEBUG_MODE) {
        cout << "=== FOLLOW集 ===" << endl;
        for (const auto& nt : nonterminals) {
            cout << "FOLLOW(" << nt << ") = { ";
            for (const auto& f : follow[nt]) cout << f << ' ';
            cout << "}" << endl;
        }
    }
}

// 计算字符串的FIRST集
bool Grammar::compute_first_of_string(const vector<string>& str, set<string>& result) {
    if (str.empty()) {
        result.insert("ε");
        return true;
    }
    
    bool all_nullable = true;
    for (size_t i = 0; i < str.size(); ++i) {
        const string& X = str[i];
        bool nullable = false;
        
        // 将FIRST(X)中除ε外的所有符号加入result
        for (const auto& f : first[X]) {
            if (f == "ε") {
                nullable = true;
            } else {
                result.insert(f);
            }
        }
        
        // 如果X不可空，则后续符号不会对FIRST(str)有贡献
        if (!nullable) {
            all_nullable = false;
            break;
        }
    }
    
    if (all_nullable) {
        result.insert("ε");
    }
    
    return all_nullable;
}

void Grammar::print_grammar() {
    cout << "=== 产生式列表 ===" << endl;
    for (size_t i = 0; i < productions.size(); ++i) {
        cout << i << ": " << productions[i].left << " → ";
        for (const auto& sym : productions[i].right) cout << sym << ' ';
        cout << endl;
    }
    cout << endl;
    cout << "=== 非终结符 ===" << endl;
    for (const auto& nt : nonterminals) cout << nt << endl;
    cout << endl;
    cout << "=== 终结符 ===" << endl;
    for (const auto& t : terminals) cout << t << endl;
    cout << endl;
    cout << "=== 起始符号 ===" << endl;
    cout << start_symbol << endl;
    cout << endl;
    cout << "=== FOLLOW集 ===" << endl;
    for (const auto& nt : nonterminals) {
        cout << "FOLLOW(" << nt << ") = { ";
        for (const auto& f : follow[nt]) cout << f << ' ';
        cout << "}" << endl;
    }
}


// LR(0)项目
struct Item {
    int production_id; // 产生式编号
    int dot_pos;       // 点的位置
    
    bool operator<(const Item& other) const {
        return tie(production_id, dot_pos) < tie(other.production_id, other.dot_pos);
    }
    
    bool operator==(const Item& other) const {
        return production_id == other.production_id && dot_pos == other.dot_pos;
    }
};

// 项目集
struct ItemSet {
    set<Item> items;
    
    bool operator<(const ItemSet& other) const {
        // 简化为直接比较两个set
        return items < other.items;
    }
    
    bool operator==(const ItemSet& other) const {
        return items == other.items;
    }

    void print_itemset(const Grammar& g, int idx) {
        cout << "项目集I" << idx << "内容:" << endl;
        for (const auto& item : items) {
            const auto& prod = g.productions[item.production_id];
            cout << "  " << prod.left << " → ";
            for (int j = 0; j < (int)prod.right.size(); ++j) {
                if (j == item.dot_pos) cout << ". ";
                cout << prod.right[j] << ' ';
            }
            if (item.dot_pos == (int)prod.right.size()) cout << ".";
            cout << " [" << item.production_id << "," << item.dot_pos << "]" << endl;
        }
    }
};

// 计算闭包
ItemSet closure(const ItemSet& I, const Grammar& g) {
    ItemSet result = I;
    queue<Item> q;
    for (const auto& item : I.items) q.push(item);

    while (!q.empty()) {
        Item item = q.front(); q.pop();
        if (item.dot_pos >= (int)g.productions[item.production_id].right.size()) continue;
        string B = g.productions[item.production_id].right[item.dot_pos];
        if (g.nonterminals.count(B)) {
            for (size_t i = 0; i < g.productions.size(); ++i) {
                if (g.productions[i].left == B) {
                    Item new_item = {(int)i, 0};
                    if (result.items.insert(new_item).second) {
                        q.push(new_item);
                    }
                }
            }
        }
    }
    return result;
}

// Goto函数
ItemSet Goto(const ItemSet& I, const string& X, const Grammar& g) {
    ItemSet goto_set;
    
    if (DEBUG_MODE) {
        cout << "  调试Goto - 对于符号 " << X << ":" << endl;
    }

    for (const auto& item : I.items) {
        const auto& prod = g.productions[item.production_id];
        
        if (DEBUG_MODE) {
            cout << "    检查项目: " << prod.left << " → ";
            for (int j = 0; j < (int)prod.right.size(); ++j) {
                if (j == item.dot_pos) cout << ". ";
                cout << prod.right[j] << ' ';
            }
            if (item.dot_pos == (int)prod.right.size()) cout << ".";
            
            bool condition1 = item.dot_pos < (int)prod.right.size();
            string right_of_dot = condition1 ? prod.right[item.dot_pos] : "END";
            bool condition2 = condition1 && right_of_dot == X;
            
            cout << " - 点位置: " << item.dot_pos << ", 产生式长度: " << prod.right.size();
            cout << ", 点后符号: " << (condition1 ? right_of_dot : "无") << ", 匹配: " << (condition2 ? "是" : "否") << endl;
        }
        
        bool has_next = item.dot_pos < (int)prod.right.size();
        if (has_next && prod.right[item.dot_pos] == X) {
            Item moved = {item.production_id, item.dot_pos + 1};
            goto_set.items.insert(moved);
        }
    }
    
    ItemSet result = closure(goto_set, g);
    
    if (DEBUG_MODE) {
        cout << "  Goto结果项目集包含 " << result.items.size() << " 个项目" << endl;
    }
    
    return result;
}

// Canonical Collection of LR(0) Items
struct CanonicalCollection {
    vector<ItemSet> C;
    map<pair<int, string>, int> transitions; // (状态编号, 符号) -> 新状态编号
};

CanonicalCollection build_canonical_collection(const Grammar& g) {
    CanonicalCollection cc;
    vector<ItemSet> C;
    map<ItemSet, int> set_id;
    queue<int> q;

    // 初始项目集
    ItemSet I0;
    I0.items.insert({0, 0});
    I0 = closure(I0, g);
    C.push_back(I0);
    set_id[I0] = 0;
    //I0.print_itemset(g, 0);

    q.push(0);
    while (!q.empty()) {
        int idx = q.front(); q.pop();
        ItemSet I = C[idx]; // 使用拷贝而非引用
        
        if (DEBUG_MODE) {
            cout << "处理状态 I" << idx << ":" << endl;
            cout << "  状态 I" << idx << " 项目集包含 " << I.items.size() << " 个项目" << endl;
            
            // 打印项目集内容
            for (const auto& item : I.items) {
                const auto& prod = g.productions[item.production_id];
                cout << "    项目: " << prod.left << " → ";
                for (int j = 0; j < (int)prod.right.size(); ++j) {
                    if (j == item.dot_pos) cout << ". ";
                    cout << prod.right[j] << ' ';
                }
                if (item.dot_pos == (int)prod.right.size()) cout << ".";
                cout << endl;
            }
        }

        // 对所有符号计算GOTO
        vector<string> symbols(g.nonterminals.begin(), g.nonterminals.end());
        symbols.insert(symbols.end(), g.terminals.begin(), g.terminals.end());
        for (const auto& X : symbols) {
            ItemSet gotoI = Goto(I, X, g);
            if (!gotoI.items.empty()) {
                // 查找是否已存在相同的项目集
                int target_id = -1;
                for (size_t i = 0; i < C.size(); i++) {
                    if (C[i].items == gotoI.items) {
                        target_id = i;
                        break;
                    }
                }
                
                if (target_id == -1) {
                    // 新项目集
                    target_id = C.size();
                    C.push_back(gotoI);
                    set_id[gotoI] = target_id;
                    q.push(target_id);
                    if (DEBUG_MODE) {
                        cout << "  添加新状态 I" << target_id << " 来自 GOTO(I" << idx << ", " << X << ")" << endl;
                    }
                } else {
                    if (DEBUG_MODE) {
                        cout << "  已存在状态 I" << target_id << " 来自 GOTO(I" << idx << ", " << X << ")" << endl;
                    }
                }
                
                cc.transitions[{idx, X}] = target_id;
            }
        }
        
    }
    cc.C = C;
    return cc;
}


// 输出项目集及内核项
void print_canonical_collection(const CanonicalCollection& cc, const Grammar& g) {
    cout << "\n=== LR(0) 项目集规范族 ===" << endl;
    for (size_t i = 0; i < cc.C.size(); ++i) {
        cout << "I" << i << ":" << endl;
        // 内核项：点不在最左侧，或是初始项目集的S'→·S
        vector<Item> kernel, closure_items;
        for (const auto& item : cc.C[i].items) {
            if (item.dot_pos > 0 || (item.production_id == 0 && item.dot_pos == 0)) {
                kernel.push_back(item);
            } else {
                closure_items.push_back(item);
            }
        }
        cout << "  [内核项]" << endl;
        for (const auto& item : kernel) {
            const auto& prod = g.productions[item.production_id];
            cout << "    " << prod.left << " → ";
            for (int j = 0; j < (int)prod.right.size(); ++j) {
                if (j == item.dot_pos) cout << ". ";
                cout << prod.right[j] << ' ';
            }
            if (item.dot_pos == (int)prod.right.size()) cout << ".";
            cout << endl;
        }
        cout << "  [闭包项]" << endl;
        for (const auto& item : closure_items) {
            const auto& prod = g.productions[item.production_id];
            cout << "    " << prod.left << " → ";
            for (int j = 0; j < (int)prod.right.size(); ++j) {
                if (j == item.dot_pos) cout << ". ";
                cout << prod.right[j] << ' ';
            }
            if (item.dot_pos == (int)prod.right.size()) cout << ".";
            cout << endl;
        }
    }
    
    cout << "\n=== 状态转移 ===" << endl;
    for (const auto& tran : cc.transitions) {
        cout << "I" << tran.first.first << " --" << tran.first.second << "--> I" << tran.second << endl;
    }
}
// ===== SLR分析表结构 =====
struct SLRAction {
    char type;  // 's' for shift, 'r' for reduce, 'a' for accept
    int value;  // state number for shift, production number for reduce

    bool operator==(const SLRAction& other) const {
        return type == other.type && value == other.value;
    }

    bool operator!=(const SLRAction& other) const {
        return !(*this == other);
    }

    bool empty() const {
        return type == '\0';
    }
};

// 输出运算符重载
ostream& operator<<(ostream& os, const SLRAction& action) {
    if (action.empty()) {
        os << "";
    } else {
        os << action.type;
        if (action.type != 'a') {  // 接受动作不需要显示值
            os << action.value;
        }
    }
    return os;
}

// SLR(1)分析表
struct SLRTable {
    // ACTION[state][terminal] = "sX"(移进), "rY"(归约), "acc"(接受), ""(空)
    map<int, map<string, SLRAction>> ACTION;
    // GOTO[state][nonterminal] = 状态编号
    map<int, map<string, int>> GOTO;
    // 冲突信息
    vector<string> conflicts;
};

SLRTable build_slr_table(const Grammar& g, const CanonicalCollection& cc) {
    SLRTable table;
    
    if (DEBUG_MODE) {
        cout << "\n=== 构建SLR(1)分析表 ===" << endl;
    }
    
    for (size_t i = 0; i < cc.C.size(); ++i) {
        const ItemSet& I = cc.C[i];
        
        if (DEBUG_MODE) {
            cout << "处理状态 I" << i << ":" << endl;
        }
        
        // 1. 归约/接受
        for (const auto& item : I.items) {
            const auto& prod = g.productions[item.production_id];
            // 归约项：点在最右
            if (item.dot_pos == (int)prod.right.size()) {
                if (prod.left == g.start_symbol) {
                    // S' → S. 接受
                    table.ACTION[i]["#"] = {'a', 0};  // 接受动作，value设为0
                    
                    if (DEBUG_MODE) {
                        cout << "  设置 ACTION[" << i << ", #] = a" << endl;
                    }
                } else {
                    // 对FOLLOW(left)内的终结符填rX
                    for (const auto& a : g.follow.at(prod.left)) {
                        SLRAction& cell = table.ACTION[i][a];
                        SLRAction act = {'r', item.production_id};
                        
                        if (DEBUG_MODE) {
                            cout << "  " << prod.left << " 的FOLLOW集包含 " << a;
                            cout << "，设置 ACTION[" << i << ", " << a << "] = " << act << endl;
                        }
                        
                        if (!cell.empty() && cell != act) {
                            string conflict = "归约冲突: 状态" + to_string(i) + ", 符号" + a + 
                                           ", " + string(1, cell.type) + to_string(cell.value) + 
                                           " vs " + string(1, act.type) + to_string(act.value);
                            table.conflicts.push_back(conflict);
                            
                            if (DEBUG_MODE) {
                                cout << " (冲突，已存在 " << cell << ")" << endl;
                            }
                        }
                        
                        cell = act;
                    }
                }
            }
        }
        
        // 2. 移进
        for (const auto& t : g.terminals) {
            auto it = cc.transitions.find({(int)i, t});
            if (it != cc.transitions.end()) {
                SLRAction& cell = table.ACTION[i][t];
                SLRAction act = {'s', it->second};
                
                if (DEBUG_MODE) {
                    cout << "  状态 I" << i << " 通过 " << t << " 转移到 I" << it->second;
                    cout << "，设置 ACTION[" << i << ", " << t << "] = " << act << endl;
                }
                
                if (!cell.empty() && cell != act) {
                    string conflict = "移进冲突: 状态" + to_string(i) + ", 符号" + t + 
                                    ", " + string(1, cell.type) + to_string(cell.value) + 
                                    " vs " + string(1, act.type) + to_string(act.value);
                    table.conflicts.push_back(conflict);
                    
                    if (DEBUG_MODE) {
                        cout << " (冲突，已存在 " << cell << ")" << endl;
                    }
                }
                
                cell = act;
            }
        }
        
        // 3. GOTO
        for (const auto& nt : g.nonterminals) {
            auto it = cc.transitions.find({(int)i, nt});
            if (it != cc.transitions.end()) {
                table.GOTO[i][nt] = it->second;
                
                if (DEBUG_MODE) {
                    cout << "  状态 I" << i << " 通过 " << nt << " 转移到 I" << it->second;
                    cout << ", 设置 GOTO[" << i << ", " << nt << "] = " << it->second << endl;
                }
            }
        }
    }
    return table;
}

void print_slr_table(const SLRTable& table, const Grammar& g, int state_count) {
    vector<string> terms(g.terminals.begin(), g.terminals.end());
    terms.push_back("#");
    vector<string> nterms(g.nonterminals.begin(), g.nonterminals.end());
    cout << "\n=== SLR(1)分析表 ===" << endl;
    cout << setw(6) << "State";
    for (const auto& t : terms) cout << setw(8) << t;
    for (const auto& nt : nterms) {
        if (nt == "S'") continue;
        cout << setw(8) << nt;
    }
    cout << endl;
    for (int i = 0; i < state_count; ++i) {
        cout << setw(6) << i;
        for (const auto& t : terms) {
            auto it = table.ACTION.find(i);
            if (it != table.ACTION.end() && it->second.count(t))
                cout << setw(8) << it->second.at(t);
            else
                cout << setw(8) << "";
        }
        for (const auto& nt : nterms) {
            if (nt == "S'") continue;
            auto it = table.GOTO.find(i);
            if (it != table.GOTO.end() && it->second.count(nt))
                cout << setw(8) << it->second.at(nt);
            else
                cout << setw(8) << "";
        }
        cout << endl;
    }
    if (!table.conflicts.empty()) {
        cout << "\n[冲突信息]" << endl;
        for (const auto& c : table.conflicts) cout << c << endl;
    }
}

#ifdef LR0_MAIN
int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--debug" || arg == "-d") {
            DEBUG_MODE = true;
        }
    }

    vector<string> rules0 = {
        //"S → E",
        "E → E + T | T",
        "T → T * F | F",
        "F → ( E ) | i"
    };
    vector<string> rules1 = {
        "S → b A S | b A",
        "A → a S c"
    };
    vector<string> rules2 = {
        "E → E + T | T",
        "T → ( E ) | a"
    };
    vector<string> rules3 = {
        "S → B B",
        "B → a B | b"
    };
    vector<string> rules4 = {
        "E → E + E | E * E | ( E ) | i"
    };   
    vector<string> rules5 = {
        "P → 𝒟 𝒮",
        "𝒟 → ε | 𝒟 D",
        "D → T d [ i ] | T d ( Â ) { 𝒟 𝒮 }",
        "T → int | float | void",
        "Â → ε | Â A",
        "A → T d | T d [ ] | T d ( T )",
        "𝒮 → S | 𝒮 S",
        "S → d = E | d [ E ] = E | if ( B ) S | if ( B ) S else S | while ( B ) S | return E | { 𝒮 } | d ( 𝑅 ) | ;",
        "E → i | f | d | d [ E ] | E + E | E * E | ( E ) | d ( 𝑅 )",
        "B → E r E | E",
        "𝑅 → ε | 𝑅 R",
        "R → E | d [ ]"
    };
    vector<string> rules6 = {
        "Prog -> DeclList",
        "DeclList -> DeclList Decl | Decl",
        "Decl -> VarDecl | FunDecl",
        "VarDecl -> Type ID SEMI | Type ID LBRACK INT_NUM RBRACK SEMI | Type ID ASG Expr SEMI",
        "Type -> INT | FLOAT | VOID",
        "FunDecl -> Type ID LPAR ParamList RPAR CompStmt",
        "ParamList -> ParamList COMMA Param | Param | ε",
        "Param -> Type ID | Type ID LBRACK RBRACK",
        "CompStmt -> LBR StmtList RBR",
        "StmtList -> StmtList Stmt | ε",
        "Stmt -> VarDecl | OtherStmt",
        "OtherStmt -> ExprStmt | CompStmt | IfStmt | LoopStmt | RetStmt | PrintStmt",
        "PrintStmt -> PRINT LPAR Expr RPAR SEMI",
        "ExprStmt -> Expr SEMI | SEMI",
        "IfStmt -> IF LPAR Expr RPAR CompStmt | IF LPAR Expr RPAR CompStmt ELSE CompStmt",
        "LoopStmt -> WHILE LPAR Expr RPAR Stmt",
        "RetStmt -> RETURN Expr SEMI | RETURN SEMI",
        "Expr -> ID ASG Expr | ID LBRACK Expr RBRACK ASG Expr | ID LPAR ArgList RPAR | SimpExpr",
        "SimpExpr -> AddExpr REL_OP AddExpr | AddExpr",
        "AddExpr -> AddExpr ADD Term | Term",
        "Term -> Term MUL Fact | Fact",
        "Fact -> ID | ID LBRACK Expr RBRACK | INT_NUM | FLOAT_NUM | LPAR Expr RPAR",
        "ArgList -> ArgList COMMA Expr | Expr | ε"
    };
    vector<string> rules7 = {
        "Prog -> DeclList",
        "DeclList -> DeclList Decl | Decl",
        "Decl -> VarDecl | FunDecl",
        "VarDecl -> Type ID SEMI | Type ID LBRACK INT_NUM RBRACK SEMI | Type ID ASG Expr SEMI",
        "Type -> INT | FLOAT | VOID",
        "FunDecl -> Type ID LPAR ParamList RPAR CompStmt",
        "ParamList -> ParamList COMMA Param | Param | ε",
        "Param -> Type ID | Type ID LBRACK RBRACK",
        "CompStmt -> LBR StmtList RBR",
        "StmtList -> StmtList Stmt | ε",
        "Stmt -> VarDecl | OtherStmt",
        "OtherStmt -> ExprStmt | CompStmt | IfStmt | LoopStmt | RetStmt",
        "ExprStmt -> Expr SEMI | SEMI",
        "IfStmt -> IF LPAR Expr RPAR CompStmt | IF LPAR Expr RPAR CompStmt ELSE CompStmt",
        "LoopStmt -> WHILE LPAR Expr RPAR Stmt",
        "RetStmt -> RETURN Expr SEMI | RETURN SEMI",
        "Expr -> ID ASG Expr | ID LBRACK Expr RBRACK ASG Expr | ID LPAR ArgList RPAR | SimpExpr",
        "SimpExpr -> AddExpr REL_OP AddExpr | AddExpr",
        "AddExpr -> AddExpr ADD Term | Term",
        "Term -> Term MUL Fact | Fact",
        "Fact -> ID | ID LBRACK Expr RBRACK | INT_NUM | FLOAT_NUM | LPAR Expr RPAR",
        "ArgList -> ArgList COMMA Expr | Expr | ε"
    };
    
    Grammar g;
    g.parse(rules0);
    
    // 首先计算FIRST集
    g.compute_first();
    
    // 然后计算FOLLOW集
    g.compute_follow();
    
    g.print_grammar();
    
    // 构建LR(0)项目集规范族
    CanonicalCollection cc = build_canonical_collection(g);
    print_canonical_collection(cc, g);

    // 构建SLR(1)分析表
    SLRTable slr = build_slr_table(g, cc);
    print_slr_table(slr, g, cc.C.size());
    return 0;
} 
#endif