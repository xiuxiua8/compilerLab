#!/usr/bin/env python3
"""
解析SLR(1)分析表并生成C++代码
"""

import re
import sys
import argparse

def parse_slr_table(content):
    """从内容中解析SLR(1)分析表"""
    # 找到SLR(1)分析表部分
    table_start = content.find("=== SLR(1)分析表 ===")
    if table_start == -1:
        print("未找到SLR(1)分析表", file=sys.stderr)
        return None, None
    
    # 提取表格部分
    table_content = content[table_start:]
    lines = table_content.split('\n')
    
    # 解析表头（终结符和非终结符）
    header_line = None
    for i, line in enumerate(lines):
        if "State" in line and not line.strip().startswith("="):
            header_line = i
            break
    
    if header_line is None:
        print("未找到表格头", file=sys.stderr)
        return None, None
    
    # 使用固定宽度解析
    header = lines[header_line]
    
    # 手动定义列名和它们的位置（基于观察）
    # 这些是根据表格格式确定的固定列
    columns_info = [
        ('State', 0, 8),
        ('ADD', 8, 16),
        ('ASG', 16, 24),
        ('COMMA', 24, 32),
        ('ELSE', 32, 40),
        ('FLOAT', 40, 48),
        ('FLOAT_NUM', 48, 56),
        ('ID', 56, 64),
        ('IF', 64, 72),
        ('INT', 72, 80),
        ('INT_NUM', 80, 88),
        ('LBR', 88, 96),
        ('LBRACK', 96, 104),
        ('LPAR', 104, 112),
        ('MUL', 112, 120),
        ('RBR', 120, 128),
        ('RBRACK', 128, 136),
        ('REL_OP', 136, 144),
        ('RETURN', 144, 152),
        ('RPAR', 152, 160),
        ('SEMI', 160, 168),
        ('VOID', 168, 176),
        ('WHILE', 176, 184),
        ('ε', 184, 192),
        ('#', 192, 200),
        ('AddExpr', 200, 208),
        ('ArgList', 208, 216),
        ('CompStmt', 216, 224),
        ('Decl', 224, 232),
        ('DeclList', 232, 240),
        ('Expr', 240, 248),
        ('ExprStmt', 248, 256),
        ('Fact', 256, 264),
        ('FunDecl', 264, 272),
        ('IfStmt', 272, 280),
        ('LoopStmt', 280, 288),
        ('OtherStmt', 288, 296),
        ('Param', 296, 304),
        ('ParamList', 304, 312),
        ('Prog', 312, 320),
        ('RetStmt', 320, 328),
        ('SimpExpr', 328, 336),
        ('Stmt', 336, 344),
        ('StmtList', 344, 352),
        ('Term', 352, 360),
        ('Type', 360, 368),
        ('VarDecl', 368, 376)
    ]
    
    # 区分终结符和非终结符
    terminals = []
    nonterminals = []
    
    for col, _, _ in columns_info[1:]:  # 跳过State列
        if col == 'ε':  # 跳过空串
            continue
        elif col == '#' or col.isupper():
            terminals.append(col)
        else:
            nonterminals.append(col)
    
    print(f"终结符: {terminals}", file=sys.stderr)
    print(f"非终结符: {nonterminals}", file=sys.stderr)
    
    # 解析表格内容
    action_table = {}
    goto_table = {}
    
    # 处理数据行
    for line in lines[header_line+1:]:
        if not line.strip() or line.strip().startswith('='):
            continue
        
        # 提取状态号
        state_str = line[:8].strip()
        if not state_str.isdigit():
            continue
            
        state = int(state_str)
        action_table[state] = {}
        goto_table[state] = {}
        
        # 根据列位置提取值
        for col, start, end in columns_info[1:]:  # 跳过State列
            if start < len(line):
                value = line[start:min(end, len(line))].strip()
                
                if value and value != '-':
                    if col in terminals:
                        # ACTION表项（s数字, r数字, acc）
                        action_table[state][col] = value
                    elif col in nonterminals:
                        # GOTO表项（纯数字）
                        if value.isdigit():
                            goto_table[state][col] = int(value)
    
    print(f"解析完成: {len(action_table)} 个状态", file=sys.stderr)
    return action_table, goto_table

def generate_cpp_code(action_table, goto_table):
    """生成C++代码来初始化SLR表"""
    code = []
    
    code.append("// 自动生成的SLR(1)分析表初始化代码")
    code.append("void IntegratedCompiler::initializeSLRTable() {")
    
    # 生成ACTION表
    code.append("    // ACTION表")
    for state in sorted(action_table.keys()):
        for terminal, action in sorted(action_table[state].items()):
            code.append(f'    slrTable.ACTION[{state}]["{terminal}"] = "{action}";')
    
    code.append("")
    code.append("    // GOTO表")
    
    # 生成GOTO表
    for state in sorted(goto_table.keys()):
        for nonterminal, goto_state in sorted(goto_table[state].items()):
            code.append(f'    slrTable.GOTO[{state}]["{nonterminal}"] = {goto_state};')
    
    code.append("}")
    
    return '\n'.join(code)

def main():
    parser = argparse.ArgumentParser(description='解析SLR(1)分析表并生成C++代码')
    parser.add_argument('input_file', nargs='?', help='输入文件（可选，默认从标准输入读取）')
    parser.add_argument('-o', '--output', default='slr_table_init.cpp', help='输出文件名（默认：slr_table_init.cpp）')
    
    args = parser.parse_args()
    
    # 读取输入
    if args.input_file:
        print(f"从文件 {args.input_file} 读取SLR表...", file=sys.stderr)
        with open(args.input_file, 'r') as f:
            content = f.read()
    elif not sys.stdin.isatty():  # 如果有管道输入
        print("从标准输入读取SLR表...", file=sys.stderr)
        content = sys.stdin.read()
    else:
        print("用法:", file=sys.stderr)
        print("  1. 从管道输入: ./lab3/lr0 | python3 parse_slr_table.py", file=sys.stderr)
        print("  2. 从文件输入: python3 parse_slr_table.py <SLR表文件>", file=sys.stderr)
        print("  3. 指定输出文件: python3 parse_slr_table.py -o output.cpp <SLR表文件>", file=sys.stderr)
        sys.exit(1)
    
    action_table, goto_table = parse_slr_table(content)
    if action_table is None:
        sys.exit(1)
    
    cpp_code = generate_cpp_code(action_table, goto_table)
    
    # 输出到文件
    with open(args.output, 'w') as f:
        f.write(cpp_code)
    
    print(f"已生成 {args.output}", file=sys.stderr)
    print(f"ACTION表项数: {sum(len(actions) for actions in action_table.values())}", file=sys.stderr)
    print(f"GOTO表项数: {sum(len(gotos) for gotos in goto_table.values())}", file=sys.stderr)

if __name__ == "__main__":
    main() 