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

#include <cmath>
#include <complex>
#include "gtest/gtest.h"
#include "qpp.h"

using namespace qpp;

// Unit testing "classes/gates.h"

/******************************************************************************/
/// BEGIN template<typename Derived> dyn_mat<typename Derived::Scalar>
///        qpp::Gates::CTRL(const Eigen::MatrixBase<Derived>& A,
///        const std::vector<idx>& ctrl,
///        const std::vector<idx>& subsys,
///        idx n,
///        idx d = 2) const
TEST(qpp_Gates_CTRL, Qubits)
{
    // CNOT control-target on 2 qubits
    cmat CTRL1 = gt.CTRL(gt.X, {0}, {1}, 2);
    EXPECT_EQ(CTRL1, gt.CNOT);

    // CNOT target-control on 2 qubits
    cmat CTRL2 = gt.CTRL(gt.X, {1}, {0}, 2);
    EXPECT_EQ(CTRL2, gt.CNOTba);

    // TOFOLI
    cmat CTRL3 = gt.CTRL(gt.X, {0, 1}, {2}, 3);
    EXPECT_EQ(CTRL3, gt.TOF);
    CTRL3 = gt.CTRL(gt.X, {0, 1}, {2}, 3, 2); // test non-default args
    EXPECT_EQ(CTRL3, gt.TOF);

    // random gate as multiple control on 2 qubits
    cmat U = randU(2);
    cmat CTRL4 = gt.CTRL(U, {0, 2}, {1}, 3);
    ket psi1 = mket({0, 0, 1});
    ket res1 = mket({0, 0, 1});
    EXPECT_NEAR(0, norm(CTRL4 * psi1 - res1), 1e-10);

    ket psi2 = mket({1, 1, 1});
    ket res2 = kron(st.z1, U * st.z1, st.z1);
    EXPECT_NEAR(0, norm(CTRL4 * psi2 - res2), 1e-10);
}

TEST(qpp_Gates_CTRL, Qudits)
{
    idx D = 3; // qutrits

    // CNOT control-target on 2 qutrits
    cmat CTRL1 = gt.CTRL(gt.Xd(3), {0}, {1}, 2, D);
    EXPECT_NEAR(0, norm(CTRL1 * mket({0, 0}, {D, D}) -
                        mket({0, 0}, {D, D})), 1e-10);
    EXPECT_NEAR(0, norm(CTRL1 * mket({0, 1}, {D, D}) -
                        mket({0, 1}, {D, D})), 1e-10);
    EXPECT_NEAR(0, norm(CTRL1 * mket({0, 2}, {D, D}) -
                        mket({0, 2}, {D, D})), 1e-10);
    EXPECT_NEAR(0, norm(CTRL1 * mket({1, 0}, {D, D}) -
                        mket({1, 1}, {D, D})), 1e-10);
    EXPECT_NEAR(0, norm(CTRL1 * mket({1, 1}, {D, D}) -
                        mket({1, 2}, {D, D})), 1e-10);
    EXPECT_NEAR(0, norm(CTRL1 * mket({1, 2}, {D, D}) -
                        mket({1, 0}, {D, D})), 1e-10);
    EXPECT_NEAR(0, norm(CTRL1 * mket({2, 0}, {D, D}) -
                        mket({2, 2}, {D, D})), 1e-10);
    EXPECT_NEAR(0, norm(CTRL1 * mket({2, 1}, {D, D}) -
                        mket({2, 0}, {D, D})), 1e-10);
    EXPECT_NEAR(0, norm(CTRL1 * mket({2, 2}, {D, D}) -
                        mket({2, 1}, {D, D})), 1e-10);

    // CNOT target-control on 2 qutrits
    cmat CTRL2 = gt.CTRL(gt.Xd(3), {1}, {0}, 2, D);
    EXPECT_NEAR(0, norm(CTRL2 * mket({0, 0}, {D, D}) -
                        mket({0, 0}, {D, D})), 1e-10);
    EXPECT_NEAR(0, norm(CTRL2 * mket({0, 1}, {D, D}) -
                        mket({1, 1}, {D, D})), 1e-10);
    EXPECT_NEAR(0, norm(CTRL2 * mket({0, 2}, {D, D}) -
                        mket({2, 2}, {D, D})), 1e-10);
    EXPECT_NEAR(0, norm(CTRL2 * mket({1, 0}, {D, D}) -
                        mket({1, 0}, {D, D})), 1e-10);
    EXPECT_NEAR(0, norm(CTRL2 * mket({1, 1}, {D, D}) -
                        mket({2, 1}, {D, D})), 1e-10);
    EXPECT_NEAR(0, norm(CTRL2 * mket({1, 2}, {D, D}) -
                        mket({0, 2}, {D, D})), 1e-10);
    EXPECT_NEAR(0, norm(CTRL2 * mket({2, 0}, {D, D}) -
                        mket({2, 0}, {D, D})), 1e-10);
    EXPECT_NEAR(0, norm(CTRL2 * mket({2, 1}, {D, D}) -
                        mket({0, 1}, {D, D})), 1e-10);
    EXPECT_NEAR(0, norm(CTRL2 * mket({2, 2}, {D, D}) -
                        mket({1, 2}, {D, D})), 1e-10);

    // multiple Control-X-X, partial testing
    cmat CTRL3 = gt.CTRL(kron(gt.Xd(3), gt.Xd(3)), {1, 4}, {2, 3}, 6, 3);
    ket psi1 = mket({0, 1, 2, 2, 1, 1}, {D, D, D, D, D, D});
    ket res1 = mket({0, 1, 0, 0, 1, 1}, {D, D, D, D, D, D});
    EXPECT_NEAR(0, norm(CTRL3 * psi1 - res1), 1e-10);

    ket psi2 = mket({0, 1, 2, 2, 2, 1}, {D, D, D, D, D, D});
    ket res2 = mket({0, 1, 2, 2, 2, 1}, {D, D, D, D, D, D});
    EXPECT_NEAR(0, norm(CTRL3 * psi2 - res2), 1e-10);

    ket psi3 = mket({1, 2, 1, 0, 2, 2}, {D, D, D, D, D, D});
    ket res3 = mket({1, 2, 0, 2, 2, 2}, {D, D, D, D, D, D});
    EXPECT_NEAR(0, norm(CTRL3 * psi3 - res3), 1e-10);
}
/******************************************************************************/
/// BEGIN template<typename Derived>
///       dyn_mat<typename Derived::Scalar> qpp::Gates::expandout(
///       const Eigen::MatrixBase<Derived>& A,
///       idx pos,
///       const std::vector<idx>& dims) const
TEST(qpp_Gates_expandout, AllTests)
{
    // single qubit (degenerate case) random gate expansion
    cmat U = randU(2);
    EXPECT_EQ(gt.expandout(U, 0, {2}), U);

    // 4 qutrits, identity on qutrit 3 expansion
    EXPECT_EQ(gt.expandout(gt.Id(3), 2, {3, 3, 3, 3}), gt.Id(81));

    // 3 qubits, X on qudit 2 expansion
    EXPECT_EQ(gt.expandout(gt.X, 1, {2, 2, 2}), kron(gt.Id2, gt.X, gt.Id2));
}
/******************************************************************************/
/// BEGIN cmat qpp::Gates::Fd(idx D) const
TEST(qpp_Gates_Fd, AllTests)
{
    EXPECT_NEAR(0, norm(gt.Fd(1) - gt.Id(1)), 1e-10);

    EXPECT_NEAR(0, norm(gt.Fd(2) - gt.H), 1e-10);

    cmat F3(3, 3);
    cplx o3 = omega(3);
    F3 << 1, 1, 1, 1, o3, o3 * o3, 1, o3 * o3, o3;
    F3 /= std::sqrt(3);
    EXPECT_NEAR(0, norm(gt.Fd(3) - F3), 1e-10);

    cmat F4(4, 4);
    cplx o4 = omega(4);
    F4 << 1, 1, 1, 1,
            1, o4, o4 * o4, o4 * o4 * o4,
            1, o4 * o4, 1, o4 * o4,
            1, o4 * o4 * o4, o4 * o4, o4;
    F4 /= std::sqrt(4);
    EXPECT_NEAR(0, norm(gt.Fd(4) - F4), 1e-10);
}
/******************************************************************************/
/// BEGIN  template<typename Derived = Eigen::MatrixXcd>
///        qpp::Gates::Id(idx D) const
TEST(qpp_Gates_Id, AllTests)
{
    EXPECT_EQ(gt.Id(1), Eigen::MatrixXcd::Identity(1, 1));
    EXPECT_EQ(gt.Id(2), Eigen::MatrixXcd::Identity(2, 2));
    EXPECT_EQ(gt.Id(3), Eigen::MatrixXcd::Identity(3, 3));
    EXPECT_EQ(gt.Id(100), Eigen::MatrixXcd::Identity(100, 100));
}
/******************************************************************************/
/// BEGIN  cmat qpp::Gates::Rn(double theta, const std::vector<double>& n) const
TEST(qpp_Gates_Rn, AllTests)
{
    // |z0> stays invariant (up to a phase) if rotated by any angle
    // around the Z axis
    EXPECT_NEAR(0, norm(st.pz0 - prj(gt.Rn(2.345, {0, 0, 1}) * st.z0)), 1e-10);

    // |z0> gets a (-1) phase if rotated by 2pi around the X axis
    EXPECT_NEAR(0, norm(st.z0 + gt.Rn(2 * pi, {1, 0, 0}) * st.z0), 1e-10);

    // |z0> gets a (-1) phase if rotated by 2pi around the Y axis
    EXPECT_NEAR(0, norm(st.z0 + gt.Rn(2 * pi, {0, 1, 0}) * st.z0), 1e-10);

    // rotate |x0> by pi/2 around the Z axis, must obtain |y0> (up to a phase)
    EXPECT_NEAR(0, norm(st.py0 - prj(gt.Rn(pi / 2, {0, 0, 1}) * st.x0)), 1e-10);

    // rotate |y0> by pi/2 around the X axis, must obtain |z0> (up to a phase)
    EXPECT_NEAR(0, norm(st.pz0 - prj(gt.Rn(pi / 2, {1, 0, 0}) * st.y0)), 1e-10);

    // rotate |z0> by pi/2 around the Y axis, must obtain |x0> (up to a phase)
    EXPECT_NEAR(0, norm(st.px0 - prj(gt.Rn(pi / 2, {0, 1, 0}) * st.z0)), 1e-10);

    // rotate |y0> by pi around the Z axis, must obtain |y1> (up to a phase)
    EXPECT_NEAR(0, norm(st.py1 - prj(gt.Rn(pi, {0, 0, 1}) * st.y0)), 1e-10);
}
/******************************************************************************/
/// BEGIN cmat qpp::Gates::Xd(idx D) const
TEST(qpp_Gates_Xd, AllTests)
{
    for (idx D = 1; D < 10; ++D)
    {
        cmat Xd = gt.Xd(D);
        for (idx i = 0; i < D; ++i)
        {
            ket psi = mket({i}, D);
            ket res = mket({(i + 1) % D}, D);
            EXPECT_NEAR(0, norm(res - Xd * psi), 1e-10);
        }
    }
}
/******************************************************************************/
/// BEGIN cmat qpp::Gates::Zd(idx D) const
TEST(qpp_Gates_Zd, AllTests)
{
    for (idx D = 1; D < 10; ++D)
    {
        cmat Zd = gt.Zd(D);
        cplx oD = omega(D);
        for (idx i = 0; i < D; ++i)
        {
            ket psi = mket({i}, D);
            ket res = std::pow(oD, i) * psi;
            EXPECT_NEAR(0, norm(res - Zd * psi), 1e-10);
        }
    }
}
/******************************************************************************/
