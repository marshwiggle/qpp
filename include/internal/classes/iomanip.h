/*
 * Quantum++
 *
 * Copyright (c) 2013 - 2016 Vlad Gheorghiu (vgheorgh@gmail.com)
 *
 * This file is part of Quantum++.
 *
 * Quantum++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Quantum++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Quantum++.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
* \file internal/classes/iomanip.h
* \brief Input/output manipulators
*/

#ifndef INTERNAL_CLASSES_IOMANIP_H_
#define INTERNAL_CLASSES_IOMANIP_H_

namespace qpp
{
namespace internal
{

// implementation details
namespace details_
{
struct Display_Impl_
{
    template<typename T>
    // T must support rows(), cols(), operator()(idx, idx)
    std::ostream& display_impl_(const T& A,
                                std::ostream& os,
                                double chop = qpp::chop) const
    {
        std::ostringstream ostr;
        ostr.copyfmt(os); // copy os' state

        std::vector<std::string> vstr;
        std::string strA;

        for (idx i = 0; i < static_cast<idx>(A.rows()); ++i)
        {
            for (idx j = 0;
                 j < static_cast<idx>(A.cols()); ++j)
            {
                strA.clear(); // clear the temporary string
                ostr.clear();
                ostr.str(std::string {}); // clear the ostringstream

                // convert to complex
                double re = static_cast<cplx>(A(i, j)).real();
                double im = static_cast<cplx>(A(i, j)).imag();

                if (std::abs(re) < chop && std::abs(im) < chop)
                {
                    ostr << "0 "; // otherwise segfault on destruction
                    // if using only vstr.push_back("0 ");
                    // bug in MATLAB libmx
                    vstr.push_back(ostr.str());
                } else if (std::abs(re) < chop)
                {
                    ostr << im;
                    vstr.push_back(ostr.str() + "i");
                } else if (std::abs(im) < chop)
                {
                    ostr << re;
                    vstr.push_back(ostr.str() + " ");
                } else
                {
                    ostr << re;
                    strA = ostr.str();

                    strA += (im > 0 ? " + " : " - ");
                    ostr.clear();
                    ostr.str(std::string()); // clear
                    ostr << std::abs(im);
                    strA += ostr.str();
                    strA += "i";
                    vstr.push_back(strA);
                }
            }
        }

        // determine the maximum lenght of the entries in each column
        std::vector<idx> maxlengthcols(A.cols(), 0);

        for (idx i = 0; i < static_cast<idx>(A.rows());
             ++i)
            for (idx j = 0;
                 j < static_cast<idx>(A.cols()); ++j)
                if (vstr[i * A.cols() + j].size() > maxlengthcols[j])
                    maxlengthcols[j] = vstr[i * A.cols() + j].size();

        // finally display it!
        for (idx i = 0; i < static_cast<idx>(A.rows()); ++i)
        {
            os << std::setw(static_cast<int>(maxlengthcols[0])) << std::right
                << vstr[i * A.cols()]; // display first column
            // then the rest
            for (idx j = 1;
                 j < static_cast<idx>(A.cols()); ++j)
                os << std::setw(static_cast<int>(maxlengthcols[j] + 2))
                    << std::right << vstr[i * A.cols() + j];

            if (i < static_cast<idx>(A.rows()) - 1)
                os << std::endl;
        }

        return os;
    }
};
} /* namespace details_ */

// ostream manipulators for nice formatting of
// Eigen matrices and STL/C-style containers/vectors

template<typename InputIterator>
class IOManipRange : public IDisplay
{
    InputIterator first_, last_;
    std::string separator_, start_, end_;
public:
    explicit IOManipRange(InputIterator first, InputIterator last,
                          const std::string& separator,
                          const std::string& start = "[",
                          const std::string& end = "]") :
            first_{first},
            last_{last},
            separator_{separator},
            start_{start},
            end_{end}
    {
    }

    // to silence -Weffc++ warnings for classes that have pointer members
    // (whenever we have a pointer instantiation,
    // i.e. iterator is a raw pointer)
    IOManipRange(const IOManipRange&) = default;

    IOManipRange& operator=(const IOManipRange&) = default;

private:
    std::ostream& display(std::ostream& os) const override
    {
        os << start_;

        bool first = true;
        for (auto it = first_; it != last_; ++it)
        {
            if (!first)
                os << separator_;
            first = false;
            os << *it;
        }
        os << end_;

        return os;
    }
}; // class IOManipRange

template<typename PointerType>
class IOManipPointer : public IDisplay
{
    const PointerType* p_;
    idx N_;
    std::string separator_, start_, end_;
public:
    explicit IOManipPointer(const PointerType* p, idx N,
                            const std::string& separator,
                            const std::string& start = "[",
                            const std::string& end = "]") :
            p_{p},
            N_{N},
            separator_{separator},
            start_{start},
            end_{end}
    {
    }

    // to silence -Weffc++ warnings for classes that have pointer members
    IOManipPointer(const IOManipPointer&) = default;

    IOManipPointer& operator=(const IOManipPointer&) = default;

private:
    std::ostream& display(std::ostream& os) const override
    {
        os << start_;

        for (idx i = 0; i < N_ - 1; ++i)
            os << p_[i] << separator_;
        if (N_ > 0)
            os << p_[N_ - 1];

        os << end_;

        return os;
    }
}; // class IOManipPointer

class IOManipEigen : public IDisplay, private details_::Display_Impl_
{
    cmat A_;
    double chop_;
public:
    // Eigen matrices
    template<typename Derived>
    explicit IOManipEigen(const Eigen::MatrixBase<Derived>& A,
                          double chop = qpp::chop) :
            A_{A.template cast<cplx>()}, // copy, so we can bind rvalues safely
            chop_{chop}
    {
    }

    // Complex numbers
    explicit IOManipEigen(const cplx z, double chop = qpp::chop) :
            A_{cmat::Zero(1, 1)}, chop_{chop}
    {
        // put the complex number inside an Eigen matrix
        A_(0, 0) = z;
    }

private:
    std::ostream& display(std::ostream& os) const override
    {
        return display_impl_(A_, os, chop);
    }
}; // class IOManipEigen

} /* namespace internal */
} /* namespace qpp */

#endif /* INTERNAL_CLASSES_IOMANIP_H_ */
