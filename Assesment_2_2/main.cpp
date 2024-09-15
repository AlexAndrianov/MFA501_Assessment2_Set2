#include <iostream>
#include <fstream>
#include <sstream>

#include "Matrix.h"

using namespace std;

math::Matrix readMatrix(std::ifstream &inputFile)
{
    math::Matrix mtx;

    std::string line;
    while (std::getline(inputFile, line)) {
        std::istringstream lineStream(line);

        std::vector<double> values;
        double value;

        while (lineStream >> value) {
            values.push_back(value);
        }

        mtx.addRow(math::Vector(std::move(values)));
    }

    return mtx;
}

int main(int argc, char* argv[])
{
    std::string toCin;
    if(argc < 2 || !argv)
    {
        cout << "Enter matrix file name to calculate the matrix determinant!" << endl;
        cin >> toCin;
        return -1;
    }

    std::ifstream inputFile(argv[1]);

    if (!inputFile.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return 1;
    }

    auto matrix = readMatrix(inputFile);

    if(matrix.isEmpty())
    {
        std::cerr << "Matrix should not be empty" << std::endl;
        return 1;
    }

    if(!matrix.isSquare())
    {
        std::cerr << "Matrix should be square" << std::endl;
        return 1;
    }

    try {
        const auto determinant = matrix.calculateDeterminantLaplaceExpansion();

        std::cout << "Determinant for matrix: " << std::endl;
        std::cout << matrix;
        std::cout << "Is: " << std::to_string(determinant) <<  std::endl;
    }
    catch(std::exception &ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
