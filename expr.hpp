#pragma once
#include <memory>
#include "basics.hpp"
#include "opcode.hpp"

namespace expr {

class Traversor;

class Expr {
    public :

    virtual void accept(Traversor&) = 0;
    virtual ~Expr() = default;
};

class IntExpr : public Expr {
    public :

    void accept(Traversor& tr) override;
    IntExpr(value::int_t n) : data(n) {}
    value::int_t data;

    ~IntExpr() = default;
};

class FloatExpr : public Expr {
    public :

    void accept(Traversor& tr) override;
    FloatExpr(value::float_t n) : data(n) {}
    value::float_t data;

    ~FloatExpr() = default;
};

class BinExpr : public Expr {
    public :

    void accept(Traversor& tr) override;
    BinExpr(token_tag tag) : tag(tag) {}

    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;
    token_tag tag;   

    ~BinExpr() = default;
};

class IdentExpr : public Expr {
    public :

    void accept(Traversor& tr) override;
    IdentExpr(std::string&& name) : data(name) {}
    
    std::string data;
    ~IdentExpr() = default;
};

class Traversor {
    public :
    virtual void visit(IntExpr&) {}
    virtual void visit(FloatExpr&) {}
    virtual void visit(IdentExpr&) {}
    virtual void visit(BinExpr&) {}
};

void IntExpr::accept(Traversor& tr) { tr.visit(*this); }
void FloatExpr::accept(Traversor& tr) { tr.visit(*this); }
void IdentExpr::accept(Traversor& tr) { tr.visit(*this); }
void BinExpr::accept(Traversor& tr) { tr.visit(*this); }
}