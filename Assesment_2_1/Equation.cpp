#include "Equation.h"

namespace math {

std::shared_ptr<Operator> UnaryOperator::derevative(TokenType t, int deep) const
{
    if(!_sub_group->isParametrique(t, deep))
        return nullptr;

    auto unaryDerevative = _sub_group->derevative(t, deep);

    switch (_t) {
    case TokenType::bracket_gr:
        return unaryDerevative;
        break;
    case TokenType::exp:
        return clone() * unaryDerevative;
        break;
    case TokenType::minus:
        return std::make_shared<UnaryOperator>(TokenType::minus, unaryDerevative);
        break;
    };

    return  nullptr;
}

std::string UnaryOperator::toString() const
{
    if(!_sub_group)
        return "";

    switch(_t)
    {
    case TokenType::minus:
    {
        if(auto sub =_sub_group->to<BinaryOperator>())
        {
            if(sub->_t != TokenType::ext)
                return g_LiteralTokens.at(_t) + "(" + _sub_group->toString() + ")";
        }
        break;
    }
    case TokenType::bracket_gr:
        return "(" + _sub_group->toString() + ")";
    }
    return g_LiteralTokens.at(_t) + _sub_group->toString();
}

std::shared_ptr<Operator> Equation::tokenToOperator(std::shared_ptr<Token> token)
{
    if(!token)
        throw std::runtime_error("Parser error.");

    if(token->_type == TokenType::var_di || token->_type == TokenType::var_mi || token->_type == TokenType::var_xi)
        return std::make_shared<VariableOperator>(token->_type);

    if(token->_type == TokenType::phi_i_1)
        return std::make_shared<Functional>(_phi_i_1);

    if(auto tokenV = std::dynamic_pointer_cast<TokenValue>(token))
    {
        if(tokenV->_v == 1.)
            return std::make_shared<OneValueOperator>();
        if(tokenV->_v == 2.)
            return std::make_shared<SquareOperator>();

        return std::make_shared<ConstantOperator>(tokenV->_v);
    }

    if(auto tokenG = std::dynamic_pointer_cast<TokenGroup>(token))
    {
        if(tokenG->_group.empty())
            return nullptr;

        switch (tokenG->_type) {
        case TokenType::bracket_gr:
            return std::make_shared<UnaryOperator>(tokenG->_type, tokenToOperator(*tokenG->_group.begin()));
        case TokenType::exp_gr:
            return std::make_shared<UnaryOperator>((*tokenG->_group.begin())->_type, tokenToOperator(*tokenG->_group.rbegin()));
        case TokenType::single_minus_gr:
            return std::make_shared<UnaryOperator>(TokenType::minus, tokenToOperator(*tokenG->_group.rbegin()));
        case TokenType::ext_gr:
            return std::make_shared<BinaryOperator>(TokenType::ext, tokenToOperator(*tokenG->_group.begin()),
                                                    tokenToOperator(*tokenG->_group.rbegin()));
        case TokenType::multipl_gr:
            return std::make_shared<BinaryOperator>((*(++(tokenG->_group.begin())))->_type, tokenToOperator(*tokenG->_group.begin()),
                                                    tokenToOperator(*tokenG->_group.rbegin()));
        case TokenType::devision_gr:
            return std::make_shared<BinaryOperator>((*(++(tokenG->_group.begin())))->_type, tokenToOperator(*tokenG->_group.begin()),
                                                    tokenToOperator(*tokenG->_group.rbegin()));
        case TokenType::plus_gr:
            return std::make_shared<BinaryOperator>((*(++(tokenG->_group.begin())))->_type, tokenToOperator(*tokenG->_group.begin()),
                                                    tokenToOperator(*tokenG->_group.rbegin()));
        case TokenType::minus_gr:
            return std::make_shared<BinaryOperator>((*(++(tokenG->_group.begin())))->_type, tokenToOperator(*tokenG->_group.begin()),
                                                    tokenToOperator(*tokenG->_group.rbegin()));
        default:
            throw std::runtime_error("Parser error.");
        }
    }

    throw std::runtime_error("Undefined token: " + std::to_string(static_cast<int>(token->_type)));
}

OperatorPtr operator+(OperatorPtr left, OperatorPtr right)
{
    return std::make_shared<BinaryOperator>(TokenType::plus, left, right);
}

OperatorPtr operator-(OperatorPtr left, OperatorPtr right)
{
    return std::make_shared<BinaryOperator>(TokenType::minus, left, right);
}

OperatorPtr operator*(OperatorPtr left, OperatorPtr right)
{
    if(left->isNearOne())
        return right;
    if(right->isNearOne())
        return left;

    return std::make_shared<BinaryOperator>(TokenType::multipl, left, right);
}

OperatorPtr operator^(OperatorPtr left, OperatorPtr right)
{
    if(right->isNearOne())
        return left;

    return std::make_shared<BinaryOperator>(TokenType::ext, left, right);
}

OperatorPtr operator/(OperatorPtr left, OperatorPtr right)
{
    return std::make_shared<BinaryOperator>(TokenType::devision, left, right);
}

}
