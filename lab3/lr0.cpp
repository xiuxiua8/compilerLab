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
    map<string, set<string>> follow;

    void parse(const vector<string>& rules);
    void compute_follow();
    void print_grammar();
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
                symbols.push_back(sym);
            }
            productions.push_back({left, symbols});
        }
    }
    // 统计终结符
    for (const auto& prod : productions) {
        for (const auto& sym : prod.right) {
            if (nonterminals.count(sym) == 0 && sym != "") {
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

void Grammar::compute_follow() {
    follow.clear();
    for (const auto& nt : nonterminals) follow[nt] = {};
    // 起始符号加入#
    follow[start_symbol].insert("#");
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& prod : productions) {
            const string& A = prod.left;
            for (size_t i = 0; i < prod.right.size(); ++i) {
                const string& B = prod.right[i];
                if (nonterminals.count(B)) {
                    // B后面所有符号的FIRST集（此处直接用终结符）
                    bool nullable = true;
                    for (size_t j = i + 1; j < prod.right.size(); ++j) {
                        const string& sym = prod.right[j];
                        if (terminals.count(sym)) {
                            if (follow[B].insert(sym).second) changed = true;
                            nullable = false;
                            break;
                        } else if (nonterminals.count(sym)) {
                            // 这里简化：只加非终结符的FIRST集中的终结符
                            // 实际应递归求FIRST集，这里假设无空串产生式
                            break;
                        }
                    }
                    if (nullable || i + 1 == prod.right.size()) {
                        for (const auto& f : follow[A]) {
                            if (follow[B].insert(f).second) changed = true;
                        }
                    }
                }
            }
        }
    }
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

// SLR(1)分析表
struct SLRTable {
    // ACTION[state][terminal] = "sX"(移进), "rY"(归约), "acc"(接受), ""(空)
    map<int, map<string, string>> ACTION;
    // GOTO[state][nonterminal] = 状态编号
    map<int, map<string, int>> GOTO;
    // 冲突信息
    vector<string> conflicts;
};

SLRTable build_slr_table(const Grammar& g, const CanonicalCollection& cc) {
    SLRTable table;
    for (size_t i = 0; i < cc.C.size(); ++i) {
        const ItemSet& I = cc.C[i];
        // 1. 归约/接受
        for (const auto& item : I.items) {
            const auto& prod = g.productions[item.production_id];
            // 归约项：点在最右
            if (item.dot_pos == (int)prod.right.size()) {
                if (prod.left == g.start_symbol) {
                    // S' → S. 接受
                    table.ACTION[i]["#"] = "acc";
                } else {
                    // 对FOLLOW(left)内的终结符填rX
                    for (const auto& a : g.follow.at(prod.left)) {
                        string& cell = table.ACTION[i][a];
                        string act = "r" + to_string(item.production_id);
                        if (!cell.empty() && cell != act) {
                            table.conflicts.push_back("归约冲突: 状态" + to_string(i) + ", 符号" + a + ", " + cell + " vs " + act);
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
                string& cell = table.ACTION[i][t];
                string act = "s" + to_string(it->second);
                if (!cell.empty() && cell != act) {
                    table.conflicts.push_back("移进冲突: 状态" + to_string(i) + ", 符号" + t + ", " + cell + " vs " + act);
                }
                cell = act;
            }
        }
        // 3. GOTO
        for (const auto& nt : g.nonterminals) {
            auto it = cc.transitions.find({(int)i, nt});
            if (it != cc.transitions.end()) {
                table.GOTO[i][nt] = it->second;
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
    for (const auto& nt : nterms) cout << setw(8) << nt;
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
    
    Grammar g;
    g.parse(rules3);

    // 构建LR(0)项目集规范族
    CanonicalCollection cc = build_canonical_collection(g);
    g.compute_follow();

    g.print_grammar();
    print_canonical_collection(cc, g);

    // 构建SLR(1)分析表
    SLRTable slr = build_slr_table(g, cc);
    print_slr_table(slr, g, cc.C.size());
    return 0;
} 