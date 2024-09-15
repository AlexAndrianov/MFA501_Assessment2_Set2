#pragma once
#include <stdexcept>
#include <vector>
#include <iostream>

namespace math {

class Vector {
public:
    Vector() = default;
    Vector(std::vector<double> &&v) : _v(std::move(v)) {}

    Vector(size_t sz)
    {
        _v.resize(sz, 0.);
    }

    double &operator[](size_t i) {
        return _v[i];
    }

    const double &operator[](size_t i) const {
        return _v[i];
    }

    friend std::ostream& operator<<(std::ostream& out, const Vector& vc)
    {
        for(auto i = 0u; i < vc.size(); i++)
            out << vc._v[i] << " ";

        out << std::endl;
        return out;
    }

    size_t size() const { return _v.size(); }

    std::vector<double> _v;
};

class Matrix {
public:
    Matrix() = default;

    Matrix(size_t row, size_t col)
    {
        if(!row || !col)
            throw std::runtime_error("Empty matrix");

        _v.resize(row, Vector(col));
    }

    void addRow(Vector &&row)
    {
        if(!_v.empty() && _v[0].size() != row.size())
            throw std::runtime_error("Numbers of columns is defferent");

        _v.emplace_back(std::move(row));
    }

    bool isEmpty() const
    {
        return _v.empty();
    }

    bool isSquare() const
    {
        if(_v.empty())
            return true;

        return _v.size() == _v[0].size();
    }

    double &operator()(size_t r, size_t c) {
        return _v[r][c];
    }

    const double &operator()(size_t r, size_t c) const {
        return _v[r][c];
    }

    friend std::ostream& operator<<(std::ostream& out, const Matrix& mtx)
    {
        for(auto i = 0u; i < mtx._v.size(); i++)
            out << mtx._v[i];

        out << std::endl;
        return out;
    }

    template<bool amongRows>
    std::pair<size_t, size_t> calculateVectorWithBiggestNumberOfZeroElements() const
    {
        size_t vectorNumber = 0;
        size_t numberOfZeroElements = 0;

        for(auto i = 0u; i < _v.size(); i++)
        {
            size_t currentNumberOfZeroElements = 0;

            for(auto j = 0u; j < _v.size(); j++)
            {
                double elem = 0;
                if constexpr(amongRows)
                    elem = (*this)(i,j);
                else
                    elem = (*this)(j,i);

                if(elem == 0.0)
                    currentNumberOfZeroElements++;
            }

            if(currentNumberOfZeroElements > numberOfZeroElements)
            {
                numberOfZeroElements = currentNumberOfZeroElements;
                vectorNumber = i;
            }
        }

        return {vectorNumber, numberOfZeroElements};
    }

    Matrix minor(size_t r, size_t c) const
    {
        Matrix res;

        for(auto i = 0; i < _v.size(); i++)
        {
            if(i == r)
                continue;

            Vector vc;

            for(auto j = 0; j < _v.size(); j++)
            {
                if(j == c)
                    continue;

                vc._v.push_back(_v[i][j]);
            }

            res.addRow(std::move(vc));
        }

        return res;
    }

    double algebraicСomplement(size_t r, size_t c) const
    {
        const auto minorRC = minor(r, c);
        const auto detMinorRC = minorRC.calculateDeterminantLaplaceExpansion();

        const auto sign = (r + c) % 2 == 0 ? 1 : -1;
        return sign * detMinorRC;
    }

    double calculateDeterminantLaplaceExpansion() const
    {
        if(_v.size() == 1)
            return _v[0][0];

        if(_v.size() == 2)
            return (_v[0][0] * _v[1][1]) - (_v[1][0] * _v[0][1]);

        // define a row or column with biggest number of zero elements
        auto [biggestRow, numZeroElemInRow] = calculateVectorWithBiggestNumberOfZeroElements<true>();
        auto [biggestCol, numZeroElemInCol] = calculateVectorWithBiggestNumberOfZeroElements<false>();

        bool alongRows = numZeroElemInRow >= numZeroElemInCol;
        auto biggestVector = alongRows ? biggestRow : biggestCol;

        double res = 0;
        for(auto i = 0; i < _v.size(); i++)
        {
            auto v = alongRows ? _v[biggestVector][i] : _v[i][biggestVector];
            if(v == 0.)
                continue;

            auto det = alongRows ? algebraicСomplement(biggestVector, i) : algebraicСomplement(i, biggestVector);
            res += v * det;
        }

        return res;
    }

    std::vector<Vector> _v;
};

}
