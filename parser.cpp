#include "parser.hpp"
#include "lexer.hpp"
#include <string>
#include <vector>
#include <map>

struct Func {
    struct Out {
        bool error;
        std::vector<std::string> out;
    };

    static Out set(std::vector<Lexer> code) {
        if (code.empty()) return {.error = true};
        bool right = false;
        bool name = false;
        std::string number1 = "0";

        std::string data[2] = {"", ""};
        std::vector<std::string> datas;

        for (size_t size = 0; size < code.size(); size++) {
            if (code[size].lexerdata == "=" && (!right)) {
                if (size + 1 < code.size()) {
                    datas.push_back("mov %0 " + code[size+1].lexerdata);
                    size++;
                }
                right = true;
                continue;
            }

            if (code[size].lexerdata == ":" && (!right && !name)) {
                name = true;
                continue;
            }

            if (!right) {
                if (name) data[1] = code[size].lexerdata;
                else data[0] += code[size].lexerdata;
            }

            if (right) {
                std::string number2 = "__ERR__";
                std::string op = "__ERR__";
                if (size + 1 < code.size()) {
                    op = code[size].lexerdata;
                    number2 = code[size+1].lexerdata;

                    if (op == "+") datas.push_back("add %" + number1 + " " + number2);
                    else if (op == "-") datas.push_back("dec %" + number1 + " " + number2);
                    else if (op == "*") datas.push_back("mul %" + number1 + " " + number2);
                    else if (op == "/") datas.push_back("div %" + number1 + " " + number2);
                }
            }
        }

        std::vector<std::string> out;
        if (name) {
            if (data[0].empty() || data[1].empty()) return {.error = true};
            out.push_back("new " + data[0] + " " + data[1] + ";");

            if (right) {
                if (data[2].empty() || (!right)) return {.error = true};
                out.push_back("set " + data[1] + ";");
                for (const auto& i : datas) out.push_back(i);
            }
        } else {
            if (data[0].empty()) return {.error = true};

            if (data[0].back() == '~') {
                out.push_back("del " + data[0].substr(0, data[0].size() - 1) + ";");
            } else {
                if (data[2].empty() || !right) return {.error = true};
                out.push_back("set " + data[0] + ";");
                for (const auto& i : datas) out.push_back(i);
            }
        }

        return {.error = false, .out = out};
    }

    struct If {
        std::string left, op1, right, op2;
    };

    static Out equal(std::vector<Lexer> code) {
        if (code.empty()) return {.error = true};

        if (code[0].lexerdata != "if") return {.error = true};
        
        code.erase(code.begin());
        std::vector<Func::If> datas;
        Func::If data;
        bool right = false;

        for (size_t size = 0; size < code.size(); size++) {
            if (code[size].lexerdata == "==" ||
                code[size].lexerdata == "!=" ||
                code[size].lexerdata == "&&" ||
                code[size].lexerdata == "||") {

                if (data.op1.empty()) {
                    data.op1 = code[size].lexerdata;
                    right = true;
                } else if (data.op2.empty()) {
                    data.op2 = code[size].lexerdata;
                    right = false;
                    datas.push_back(data);
                    data.op1 = "";
                    data.op2 = "";
                    data.left = "";
                    data.right = "";
                }
                continue;
            }

            if (right) data.right += code[size].lexerdata;
            else data.left += code[size].lexerdata;
        }

        if (!data.left.empty() || !data.right.empty() || !data.op1.empty() || !data.op2.empty()) datas.push_back(data);

        std::vector<std::string> out = {"if("};
        std::string dataa;
        std::string opp2 = "&&";

        for (auto& [left, op1, right, op2] : datas) {
            if (right.empty()) right = "1";
            out.push_back("cmp " + left + " " + right + ";");
            if (opp2 == "||") {
                if (op1 == "==") out.push_back("je true;");
                else if (op1 == "!=") out.push_back("jne true;");
                else if (op1 == "&&") out.push_back("and true;");
                else if (op1 == "||") out.push_back("or true;");

                if (!opp2.empty() && opp2 != op2) out.push_back("jmp false;");
                else opp2 = op2;
            } else if (opp2 == "&&") {
                if (op1 == "==" || op1 == "&&") out.push_back("jne false;");
                else if (op1 == "!=" || op1 == "||") out.push_back("je false;");

                if (!opp2.empty() && opp2 != op2) out.push_back("jmp true;");
                else opp2 = op2;
            }
            if (!op2.empty()) opp2 = op2;
        }

        out.push_back("){");

        return {.error = false, .out = out};
    }

    static Out loop(std::vector<Lexer> code) {
        if (code.empty()) return {.error = true};
        
        if (code[0].lexerdata != "while") return {.error = true};
        
        code.erase(code.begin());
        std::vector<Func::If> datas;
        Func::If data;
        bool right = false;

        for (size_t size = 0; size < code.size(); size++) {
            if (code[size].lexerdata == "==" ||
                code[size].lexerdata == "!=" ||
                code[size].lexerdata == "&&" ||
                code[size].lexerdata == "||") {

                if (data.op1.empty()) {
                    data.op1 = code[size].lexerdata;
                    right = true;
                } else if (data.op2.empty()) {
                    data.op2 = code[size].lexerdata;
                    right = false;
                    datas.push_back(data);
                    data.op1 = "";
                    data.op2 = "";
                    data.left = "";
                    data.right = "";
                }
                continue;
            }

            if (right) data.right += code[size].lexerdata;
            else data.left += code[size].lexerdata;
        }

        if (!data.left.empty() || !data.right.empty() || !data.op1.empty() || !data.op2.empty()) datas.push_back(data);

        std::vector<std::string> out = {"while("};
        std::string dataa;
        std::string opp2 = "&&";

        for (auto& [left, op1, right, op2] : datas) {
            if (right.empty()) right = "1";
            out.push_back("cmp " + left + " " + right + ";");
            if (opp2 == "||") {
                if (op1 == "==") out.push_back("je true;");
                else if (op1 == "!=") out.push_back("jne true;");
                else if (op1 == "&&") out.push_back("and true;");
                else if (op1 == "||") out.push_back("or true;");

                if (!opp2.empty() && opp2 != op2) out.push_back("jmp false;");
                else opp2 = op2;
            } else if (opp2 == "&&") {
                if (op1 == "==" || op1 == "&&") out.push_back("jne false;");
                else if (op1 == "!=" || op1 == "||") out.push_back("je false;");

                if (!opp2.empty() && opp2 != op2) out.push_back("jmp true;");
                else opp2 = op2;
            }
            if (!op2.empty()) opp2 = op2;
        }

        out.push_back("){");

        return {.error = false, .out = out};
    }

    static Out keywords(std::vector<Lexer> code) {
        if (code.empty()) return {.error = true};

        if (code[0].lexerdata == "end") return {.error = false, .out = {"};"}};
        if (code[0].lexerdata == "else") return {.error = false, .out = {"}false{"}};

        return {.error = true};
    }
} static Func;

std::vector<std::string> parserfunc(std::map<int, std::vector<Lexer>> code) {
    std::vector<std::string> parserlayer1;

    for (const auto& [line, parser] : code) {
        Func::Out token;

        token = Func::set(parser);
        if (!token.error) {
            for (const auto& i : token.out) parserlayer1.push_back(i);
            continue;
        }

        token = Func::equal(parser);
        if (!token.error) {
            for (const auto& i : token.out) parserlayer1.push_back(i);
            continue;
        }

        token = Func::loop(parser);
        if (!token.error) {
            for (const auto& i : token.out) parserlayer1.push_back(i);
            continue;
        }

        token = Func::keywords(parser);
        if (!token.error) {
            for (const auto& i : token.out) parserlayer1.push_back(i);
            continue;
        }

        if (token.error) return {"ERROR"};
        for (const auto& i : token.out) parserlayer1.push_back(i);
    }

    std::vector<std::string> out;

    for (size_t size = 0; size < parserlayer1.size(); size++) {
        // here...
        // Şimdilik böyle:
        out.push_back(parserlayer1[size]);
    }

    return out;
}
