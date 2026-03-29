#include <chrono>
#include <iostream>
#include "parser/parser.hpp"
#include "expr.hpp"

class Printer : public expr::Traversor {

    void visit(expr::BinExpr& node) override {
        node.lhs->accept(*this);
        node.rhs->accept(*this);

        std::cout << tell_info(node.tag).name << " ";
    }

    void visit(expr::FloatExpr& node) override {
        std::cout << node.data << " ";
    }

    void visit(expr::IntExpr& node) override {
        std::cout << node.data << " ";
    }

    void visit(expr::IdentExpr& node) override {
        std::cout << node.data << " ";
    }
};

bool feed(std::string& ln_buf) {
    ln_buf.clear();
    std::cout << ">>>>> ";
    std::getline(std::cin, ln_buf);
    return true;
}

int main() {
    int abc;

    Printer printer;
    ParserState ps;
    ps.feed = feed;
    try {
        auto root = parse_expr(ps);
        if(!root) {
            std::cerr << "ROOT EMPTY\n";
        } else {
            root->accept(printer);
            std::cout << std::endl;
        }

    } catch(std::exception& e) {
        std::cerr << "ERROR : " << e.what() << std::endl;
    }
}