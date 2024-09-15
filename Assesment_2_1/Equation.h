#pragma once
#include <functional>
#include <memory>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <list>
#include <stack>
#include <deque>
#include <math.h>

namespace math {

enum class TokenType : int {
    exp = 0,
    ext,
    plus,
    minus,
    multipl,
    devision,
    left_bracket,
    right_bracket,
    value,
    var_xi,
    var_mi,
    var_di,
    all,
    any,
    bracket_gr,
    exp_gr,
    ext_gr,
    plus_gr,
    minus_gr,
    multipl_gr,
    devision_gr,
    single_minus_gr,
    nothing,
    phi_i_1,
};

const static std::unordered_map<std::string, TokenType> g_TokenLiterals = {
    {"exp", TokenType::exp },
    {"^", TokenType::ext },
    {"+", TokenType::plus },
    {"-", TokenType::minus },
    {"*", TokenType::multipl },
    {"/", TokenType::devision },
    {"(", TokenType::left_bracket },
    {")", TokenType::right_bracket },
    {"xi", TokenType::var_xi },
    {"mi", TokenType::var_mi },
    {"di", TokenType::var_di },
    {"phi(i-1)", TokenType::phi_i_1}
};

const static std::unordered_map<TokenType, std::string> g_LiteralTokens = [](){
    std::unordered_map<TokenType, std::string> res;

    for(const auto &tokenToLiteral : g_TokenLiterals)
        res[tokenToLiteral.second] = tokenToLiteral.first;

    return res;
}();

class CalculationContext {
public:
    CalculationContext(double parameterV)
        : _parameterV(parameterV) {}

    virtual ~CalculationContext() = default;

    double _parameterV = 0;
};

class Operator {
public:
    virtual ~Operator() = default;

    virtual double produce(const CalculationContext &context) const = 0;
    virtual std::string toString() const = 0;
    virtual bool isParametrique(TokenType paramT, int deep = 0) const = 0;
    virtual std::shared_ptr<Operator> clone() const = 0;
    virtual std::shared_ptr<Operator> derevative(TokenType t, int deep = 0) const = 0;
    virtual bool isNearOne() const { return false; }
    virtual void addDeep() {}

    template<class T>
    const T* to() const
    {
        return dynamic_cast<const T*>(this);
    }
};

using OperatorPtr = std::shared_ptr<Operator>;

OperatorPtr operator+(OperatorPtr left, OperatorPtr right);
OperatorPtr operator-(OperatorPtr left, OperatorPtr right);
OperatorPtr operator*(OperatorPtr left, OperatorPtr right);
OperatorPtr operator^(OperatorPtr left, OperatorPtr right);
OperatorPtr operator/(OperatorPtr left, OperatorPtr right);

struct Token {
    Token(TokenType type) : _type(type) {}
    virtual ~Token() = default;

    TokenType _type;
};

static const double tol = 1e-6;

class ConstantOperator : public Operator{
public:

    ConstantOperator(double v) : _v(v) {}

    virtual bool isNearOne() const
    {
        return fabs(1. - _v) <= tol;
    }

    virtual double produce(const CalculationContext &) const
    {
        return _v;
    }

    virtual std::string toString() const
    {
        if(fabs(std::round(_v) - _v) <= tol)
            return std::to_string(std::lround(_v));

        return std::to_string(_v);
    }

    virtual bool isParametrique(TokenType, int) const
    {
        return false;
    }

    virtual std::shared_ptr<Operator> clone() const
    {
        return std::make_shared<ConstantOperator>(_v);
    }

    virtual std::shared_ptr<Operator> derevative(TokenType, int) const
    {
        return nullptr;
    }

    double _v = 0;
};

class OneValueOperator : public ConstantOperator {
public:
    OneValueOperator() : ConstantOperator(1) {}

    virtual bool isNearOne() const { return true; }

    virtual std::string toString() const
    {
        return "1";
    }
};

class SquareOperator : public ConstantOperator {
public:
    SquareOperator() : ConstantOperator(2) {}

    virtual std::string toString() const
    {
        return "2";
    }
};

class VariableOperator : public Operator{
public:

    VariableOperator(TokenType t, int deep = 0) : _t(t), _deep(deep) { }

    virtual double produce(const CalculationContext &context) const
    {
        return context._parameterV;
    }

    virtual std::string toString() const
    {
        std::string token = g_LiteralTokens.at(_t);
        if(!_deep)
            return token;

        return *token.begin() + std::string("(i-") + std::to_string(_deep) + std::string(")");
    }

    virtual bool isParametrique(TokenType paramT, int deep) const
    {
        return _deep == deep && _t == paramT;
    }

    virtual std::shared_ptr<Operator> clone() const
    {
        return std::make_shared<VariableOperator>(_t, _deep);
    }

    virtual std::shared_ptr<Operator> derevative(TokenType t, int deep) const
    {
        return t == _t && _deep == deep ? std::make_shared<OneValueOperator>() : nullptr;
    }

    virtual void addDeep() { _deep++; }

    TokenType _t;
    int _deep = 0;
};

class UnaryOperator : public Operator {
public:

    UnaryOperator(TokenType t,
                  std::shared_ptr<Operator> sub_group)
        : _t(t), _sub_group(sub_group)
    {
        switch (_t) {
            case TokenType::bracket_gr:
                _action = [](double v){ return v; };
                break;
            case TokenType::exp_gr:
                _action = [](double v){ return exp(v); };
                break;
            case TokenType::minus:
                _action = [](double v){ return -v; };
                break;
        };
    }

    virtual double produce(const CalculationContext &context) const
    {
        return _action(_sub_group->produce(context));
    }

    virtual std::string toString() const;

    virtual bool isParametrique(TokenType paramT, int deep) const
    {
        return _sub_group->isParametrique(paramT, deep);
    }

    virtual std::shared_ptr<Operator> clone() const
    {
        return std::make_shared<UnaryOperator>(_t, _sub_group->clone());
    }

    virtual std::shared_ptr<Operator> derevative(TokenType t, int deep) const;

    virtual void addDeep() { _sub_group->addDeep(); }

    TokenType _t;
    std::shared_ptr<Operator> _sub_group;
    std::function<double(double v)> _action;
};

class BinaryOperator : public Operator{
public:

    BinaryOperator(TokenType t,
                   std::shared_ptr<Operator> left,
                   std::shared_ptr<Operator> right) :
        _t(t), _left(left), _right(right)
    {
        switch (_t) {
            case TokenType::ext:
                _action = [](double lv, double rv){ return pow(lv, rv); };
                break;
            case TokenType::multipl:
                _action = [](double lv, double rv){ return lv * rv; };
                break;
            case TokenType::devision:
                _action = [](double lv, double rv){ return lv/rv; };
                break;
            case TokenType::plus:
                _action = [](double lv, double rv){ return lv + rv; };
                break;
            case TokenType::minus:
                _action = [](double lv, double rv){ return lv - rv; };
                break;
        };
    }

    virtual double produce(const CalculationContext &context) const
    {
        const auto lVl = _left->produce(context);
        const auto rVl = _right->produce(context);

        return _action(lVl, rVl);
    }

    virtual std::string toString() const
    {
        std::string lBr;
        std::string rBr;

        /*switch (_t) {
        case TokenType::plus:
        case TokenType::minus:
            lBr = "(";
            rBr = ")";
        };*/

        return lBr + _left->toString() + g_LiteralTokens.at(_t) + _right->toString() + rBr;
    }

    virtual bool isParametrique(TokenType paramT, int deep) const
    {
        return _left->isParametrique(paramT, deep) || _right->isParametrique(paramT, deep);
    }

    virtual std::shared_ptr<Operator> clone() const
    {
        return std::make_shared<BinaryOperator>(_t, _left->clone(), _right->clone());
    }

    virtual std::shared_ptr<Operator> derevative(TokenType t, int deep) const
    {
        std::shared_ptr<Operator> l = _left->isParametrique(t, deep) ? _left->derevative(t, deep) : nullptr;
        std::shared_ptr<Operator> r = _right->isParametrique(t, deep) ? _right->derevative(t, deep) : nullptr;

        switch(_t)
        {
        case TokenType::ext:
        {
            if(r)
                throw std::runtime_error("This math operator isn't supported");

            if(!l)
                return nullptr;

            auto constOperator = std::dynamic_pointer_cast<ConstantOperator>(_right);
            if(!constOperator)
                throw std::runtime_error("This math operator isn't supported");

            // workaround to simplify output
            if(constOperator->to<OneValueOperator>())
                return l;
            if(constOperator->to<SquareOperator>())
            {
                if(auto leftUnaryOp = _left->to<UnaryOperator>())
                {
                    auto subGr = leftUnaryOp->_sub_group->to<BinaryOperator>();
                    if(subGr && (subGr->_t == TokenType::plus || subGr->_t == TokenType::minus))
                    {
                        auto llc = subGr->_left->clone();
                        auto rrc = subGr->_right->clone();

                        auto llc_sqare = llc ^ std::make_shared<SquareOperator>();
                        auto rrc_sqare = rrc ^ std::make_shared<SquareOperator>();
                        auto two_ab = std::make_shared<SquareOperator>() * llc * rrc;

                        auto fsumm = subGr->_t == TokenType::plus ? llc_sqare + two_ab : llc_sqare - two_ab;
                        return (fsumm + rrc_sqare)->derevative(t, deep);
                    }
                }
            }

            return constOperator->clone() * (_left->clone() ^ std::make_shared<ConstantOperator>(constOperator->_v - 1.)) * l;
        }
        case TokenType::plus:
        {
            if(!l)
                return r;
            if(!r)
                return l;

            return l + r;
        }
        case TokenType::minus:
        {
            if(!l)
            {
                if(!r)
                    return nullptr;

                return std::make_shared<UnaryOperator>(TokenType::minus, r);
            }
            if(!r)
                return l;

            return l - r;
        }
        case TokenType::multipl:
        {
            if(!l)
            {
                if(!r)
                    return nullptr;

                return _left->clone() * r;
            }

            if(!r)
                return l *_right->clone();

            return (l * _right->clone()) + (_left->clone() * r);
        }
        case TokenType::devision:
        {
            std::shared_ptr<Operator> top;

            if(!l)
            {
                if(!r)
                {
                    return nullptr;
                }

                top = std::make_shared<UnaryOperator>(TokenType::minus, _left->clone() * r);
            }
            else if(!r)
            {
                top = l * _right->clone();
            }
            else
            {
                top = (l * _right->clone()) - (_left->clone() * r);
            }

            return top / (_right->clone() ^ std::make_shared<SquareOperator>());
        }
        }

        throw std::runtime_error("Unsupported operator for derevative");
    }

    virtual void addDeep() { _left->addDeep(); _right->addDeep(); }

    TokenType _t;
    std::shared_ptr<Operator> _left;
    std::shared_ptr<Operator> _right;
    std::function<double(double lV, double rV)> _action;
};

class Functional : public Operator{
public:

    Functional(std::shared_ptr<Operator> sintaxis_tree_root)
        : _sintaxis_tree_root(sintaxis_tree_root) {}

    virtual double produce(const CalculationContext &c) const
    {
        return _sintaxis_tree_root->produce(c);
    }

    virtual std::string toString() const
    {
        return _sintaxis_tree_root->toString();
    }

    virtual bool isParametrique(TokenType t, int deep) const
    {
        return _sintaxis_tree_root->isParametrique(t, deep);
    }

    virtual std::shared_ptr<Operator> clone() const
    {
        return _sintaxis_tree_root->clone();
    }

    virtual std::shared_ptr<Operator> derevative(TokenType t, int deep) const
    {
        return _sintaxis_tree_root->derevative(t, deep);
    }

    virtual void addDeep() { _sintaxis_tree_root->addDeep(); }

    std::shared_ptr<Operator> _sintaxis_tree_root;
};

struct TokenValue : public Token {
    TokenValue(TokenType type, double v) : Token(type), _v(v) {}

    double _v = 0.;
};

struct TokenGroup : public Token {
    TokenGroup(TokenType type) : Token(type) {}

    std::list<std::shared_ptr<Token>> _group;
};

class Equation {

    std::shared_ptr<Operator> tokenToOperator(std::shared_ptr<Token> token);

    std::list<std::shared_ptr<Token>> tokenize(const std::string &script) const
    {
        std::list<std::shared_ptr<Token>> res;

        bool isValue = false;
        std::string buffer;

        for(auto ch : script)
        {
            if(std::isdigit(ch) || ch == '.' || ch == ',')
            {
                if(buffer.empty())
                    isValue = true;
            }
            else if(isValue)
            {
                res.emplace_back(std::make_shared<TokenValue>(TokenType::value, std::stod(buffer)));
                isValue = false;
                buffer.clear();
            }

            buffer.push_back(ch);

            auto it = g_TokenLiterals.find(buffer);
            if(it != g_TokenLiterals.end())
            {
                res.emplace_back(std::make_shared<Token>(it->second));
                buffer.clear();
            }
        }

        if(isValue)
        {
            res.emplace_back(std::make_shared<TokenValue>(TokenType::value, std::stod(buffer)));
        }
        else if(!buffer.empty())
            throw std::runtime_error("Undefined symbol: " + buffer);

        return res;
    }

    std::list<std::shared_ptr<Token>> grammatics_rules_apply(std::list<std::shared_ptr<Token>> tokens)
    {
        struct Grammatic {
            TokenType _grType;
            std::deque<TokenType> _tokenOrdering;
        };

        // order is valuable
        static const std::list<Grammatic> s_Grammatics = {
            { TokenType::bracket_gr, {TokenType::left_bracket, TokenType::all, TokenType::right_bracket} },
            { TokenType::exp_gr, {TokenType::exp, TokenType::bracket_gr}},
            { TokenType::ext_gr, {TokenType::bracket_gr, TokenType::ext, TokenType::any}},
            { TokenType::single_minus_gr, {TokenType::nothing, TokenType::minus, TokenType::any}},
            { TokenType::single_minus_gr, {TokenType::plus, TokenType::minus, TokenType::any}},
            { TokenType::single_minus_gr, {TokenType::multipl, TokenType::minus, TokenType::any}},
            { TokenType::single_minus_gr, {TokenType::minus, TokenType::minus, TokenType::any}},
            { TokenType::single_minus_gr, {TokenType::devision, TokenType::minus, TokenType::any}},
            { TokenType::single_minus_gr, {TokenType::ext, TokenType::minus, TokenType::any}},
            { TokenType::ext_gr, {TokenType::any, TokenType::ext, TokenType::any}},
            { TokenType::multipl_gr, {TokenType::any, TokenType::multipl, TokenType::any}},
            { TokenType::devision_gr, {TokenType::any, TokenType::devision, TokenType::any}},
            { TokenType::plus_gr, {TokenType::any, TokenType::plus, TokenType::any}},
            { TokenType::minus_gr, {TokenType::any, TokenType::minus, TokenType::any}}
        };

        auto ruleProcessor = [this, &tokens](const Grammatic &rule){
            std::stack<TokenType> expectedTokens;
            TokenType expectedToken;

            decltype(tokens.end()) firstSubToken;
            decltype(tokens.end()) endSubToken;

            auto resetExpectation = [&](){
                expectedTokens = std::stack<TokenType>(rule._tokenOrdering);
                expectedToken = expectedTokens.top();
                expectedTokens.pop();

                firstSubToken = tokens.end();
                endSubToken = tokens.end();
            };

            resetExpectation();

            bool allMode = false;
            auto sub_bracket_index = 0;
            TokenType bracket_type;

            for(auto rit = tokens.rbegin(); rit != tokens.rend(); rit++)
            {
                const auto currentNodeType = (*rit)->_type;

                if(expectedToken != TokenType::any && currentNodeType != expectedToken)
                {
                    if(!allMode && rule._tokenOrdering.size() != expectedTokens.size() + 1)
                        resetExpectation();

                    if(allMode && currentNodeType == bracket_type)
                        sub_bracket_index++;

                    continue;
                }

                if(expectedTokens.empty())
                {
                    if(allMode && sub_bracket_index)
                    {
                        sub_bracket_index--;
                        continue;
                    }

                    firstSubToken = rit.base();
                    --firstSubToken;

                    if(rule._grType == TokenType::single_minus_gr)
                        firstSubToken++;

                    break;
                }

                if(rule._tokenOrdering.size() == expectedTokens.size() + 1)
                {
                    endSubToken = rit.base();
                }

                auto previousToken = expectedToken;
                expectedToken = expectedTokens.top();
                expectedTokens.pop();

                if(expectedToken == TokenType::all)
                {
                    bracket_type = previousToken;

                    expectedToken = expectedTokens.top();
                    expectedTokens.pop();

                    allMode = true;
                    sub_bracket_index = 0;
                }
            }

            if(expectedToken == TokenType::nothing)
                firstSubToken = tokens.begin();

            if(firstSubToken == tokens.end())
                return false;

            auto group = std::make_shared<TokenGroup>(rule._grType);

            if(allMode)
            {
                auto firstFactSubToken = firstSubToken;
                auto endFactSubToken = endSubToken;

                group->_group = grammatics_rules_apply(std::list<std::shared_ptr<Token>>{++firstFactSubToken, --endFactSubToken});
            }
            else
            {
                group->_group = std::list<std::shared_ptr<Token>>{firstSubToken, endSubToken};
            }

            tokens.erase(firstSubToken, endSubToken);
            tokens.insert(endSubToken, group);
            return true;
        };

        for(const auto &rule : s_Grammatics)
        {
            while(ruleProcessor(rule));
        }

        return tokens;
    }

public:

    Equation(std::shared_ptr<Operator> phi_i_1 = nullptr, bool use_old_parameters = false)
        : _phi_i_1(phi_i_1)
    {
        if(use_old_parameters)
            _phi_i_1->addDeep();
    }

    void parse(const std::string &script)
    {
        auto tokens = tokenize(script);
        tokens = grammatics_rules_apply(tokens);

        if(tokens.size() != 1)
            throw std::runtime_error("Mistaken equation: " + script);

        _sintaxis_tree_root = tokenToOperator(*tokens.begin());
    }

    std::shared_ptr<Operator> _sintaxis_tree_root;
    std::shared_ptr<Operator> _phi_i_1;
};

}
