#include <iostream>
#include <fstream>

#include "Equation.h"

using namespace std;

void equations_test()
{
    auto testEq = [](auto eqString, auto derBy){
        math::Equation eq1;
        eq1.parse(eqString);

        std::cout << "Equation: " << eq1._sintaxis_tree_root->toString() << std::endl;
        auto dv = eq1._sintaxis_tree_root->derevative(derBy);
        std::cout << "Has derevative: " << (dv ? dv->toString() : "zero") << std::endl;
    };

    testEq("1", math::TokenType::var_xi);
    testEq("2*xi", math::TokenType::var_xi);
    testEq("xi^2", math::TokenType::var_xi);
    testEq("2*xi^2", math::TokenType::var_xi);
    testEq("xi+2*xi^2", math::TokenType::var_xi);
    testEq("exp(xi)", math::TokenType::var_xi);
    testEq("exp(xi^3+xi^2+xi)", math::TokenType::var_xi);
    testEq("-(xi-mi)^2", math::TokenType::var_xi);
    testEq("(-(xi-mi)^2)/(2*di^2)", math::TokenType::var_xi);
    testEq("(-(xi-mi)^2)/(2*di^2)", math::TokenType::var_di);
    testEq("(-(xi-mi)^2)/(2*di^2)", math::TokenType::var_mi);
    testEq("exp((-(xi-mi)^2)/(2*di^2))", math::TokenType::var_xi);
    testEq("exp((-(xi-mi)^2)/(2*di^2))", math::TokenType::var_di);
    testEq("exp((-(xi-mi)^2)/(2*di^2))", math::TokenType::var_mi);

    std::string dummy;
    std::cin >> dummy;
}

void iterable_equations_test()
{
    const auto i_number_iterations = 2;
    const auto derBy = math::TokenType::var_mi;

    math::Equation eq;
    eq.parse("exp((-(xi-mi)^2)/(2*di^2))");

    std::cout << "Equation: " << eq._sintaxis_tree_root->toString() << std::endl;
    auto dv = eq._sintaxis_tree_root->derevative(derBy);
    std::cout << "Has derevative: " << (dv ? dv->toString() : "zero") << std::endl;

    for(auto i = 0; i < i_number_iterations; i++)
    {
        math::Equation eqNextStep(eq._sintaxis_tree_root, true);
        eqNextStep.parse("exp((-(xi-mi)^2)/(2*di^2))+exp((-(phi(i-1)-mi)^2)/(2*di^2))");

        std::cout << "Equation: " << eqNextStep._sintaxis_tree_root->toString() << std::endl;
        auto dv = eqNextStep._sintaxis_tree_root->derevative(derBy);
        std::cout << "Has derevative: " << (dv ? dv->toString() : "zero") << std::endl;
    }
}

int main(int argc, char* argv[])
{
    if(argc < 2 || !argv)
    {
        cout << "Enter the number of gradient iterations and the parameter" << endl;
        return -1;
    }

    if(std::string(argv[1]) == "test")
    {
        equations_test();
        iterable_equations_test();
        return 0;
    }

    if(argc < 4)
    {
        cout << "Enter the number of gradient iterations, derevatives parameter and calculating type (parameters are same for all iterations or not)" << endl;
        return -1;
    }

    const auto numberOfIterations = std::stoi(argv[1]);
    const std::string byParameter = argv[2];
    const bool parameters_are_same_for_all_iterations = std::stoi(argv[3]);

    cout << "Program will calculate gradients by " << (byParameter == "d" ? "standard deviations" : "centres") << " parameter, and "
         << numberOfIterations << " iterations, also derevative parameters are "
         << (parameters_are_same_for_all_iterations ? "same" : "not same")
         << " for all iterations." << std::endl;

    auto derBy = math::TokenType::var_mi;
    if(byParameter == "d")
        derBy = math::TokenType::var_di;

    math::Equation phi0;
    phi0.parse("exp((-(xi-mi)^2)/(2*di^2))");

    auto phi_previous = phi0._sintaxis_tree_root;

    if(parameters_are_same_for_all_iterations)
    {
        cout << "Phi0 equation: " << phi0._sintaxis_tree_root->toString() << std::endl;

        for(auto i = 0; i < numberOfIterations; i++)
        {
            cout << "!!!! Iteration " << i << " !!!!" << std::endl;
            math::Equation eqNextStep(phi_previous, false);
            eqNextStep.parse("exp((-(xi-mi)^2)/(2*di^2))+exp((-(phi(i-1)-mi)^2)/(2*di^2))");

            std::cout << "Equation: " << eqNextStep._sintaxis_tree_root->toString() << std::endl;
            auto dv = eqNextStep._sintaxis_tree_root->derevative(derBy);
            std::cout << "Has gradient by selected parameter: " << (dv ? dv->toString() : "zero") << std::endl;

            phi_previous = eqNextStep._sintaxis_tree_root;
        }
    }
    else
    {
        for(auto i = 0; i < numberOfIterations; i++)
        {
            math::Equation eqNextStep(phi_previous, true);
            eqNextStep.parse("exp((-(xi-mi)^2)/(2*di^2))+exp((-(phi(i-1)-mi)^2)/(2*di^2))");
            phi_previous = eqNextStep._sintaxis_tree_root;
        }

        math::Equation finalEquation;
        finalEquation._sintaxis_tree_root = phi_previous;

        cout << "Equation : Phii = " << finalEquation._sintaxis_tree_root->toString() << std::endl;
        cout << "Has next derivatives: " << std::endl;

        for(auto i = 0; i <= numberOfIterations; i++)
        {
            std::string parameter = byParameter;
            if(!i)
                parameter += "i";
            else
            {
                parameter += "(i-" + std::to_string(i) + ")";
            }

            cout << "By parameter: " << parameter << ": dPhii/d" << parameter << " = "<< std::endl;

            auto dv = finalEquation._sintaxis_tree_root->derevative(derBy, i);
            std::cout << (dv ? dv->toString() : "zero") << std::endl;
        }
    }

    return 0;
}
