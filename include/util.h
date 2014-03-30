/* 
 * File:   util.h
 * Author: vlad
 *
 * Created on December 12, 2013, 10:41 PM
 */

#ifndef UTIL_H_
#define	UTIL_H_

#include <stdexcept>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "types.h"
#include "constants.h"
#include "internal.h"
#include "stat.h"
#include "io.h"
#include "functional.h"

// utility functions

namespace qpp
{
// Eigen function wrappers (inlines)

// transpose, preserve return type
template<typename MatrixType>
types::TemplatedEigenMatrix<MatrixType> transpose(
		const types::EigenExpression<MatrixType>& A)
{
	return A.transpose();
}

// conjugate, preserve return type
template<typename MatrixType>
types::TemplatedEigenMatrix<MatrixType> conjugate(
		const types::EigenExpression<MatrixType>& A)
{
	return A.conjugate();
}

// adjoint, preserve return type
template<typename MatrixType>
types::TemplatedEigenMatrix<MatrixType> adjoint(
		const types::EigenExpression<MatrixType>& A)
{
	return (A).adjoint();
}

// trace, preserve return type
template<typename MatrixType>
typename MatrixType::Scalar trace(const types::EigenExpression<MatrixType>& A)
{
	return A.trace();
}

// functor; Apply f(A) component-wise, where (*f) is the function pointer
template<typename FunctionInputType, typename FunctionOutputType,
		typename MatrixInputType>
Eigen::Matrix<FunctionOutputType, Eigen::Dynamic, Eigen::Dynamic> fun(
		const types::EigenExpression<MatrixInputType> &A,
		FunctionOutputType (*f)(const FunctionInputType &))
// The type of A is MatrixInputType
// The function is of the form FunctionOutputType f(const FunctionInputType &)
// The output is an Eigen::Matrix of the type FunctionOutputType

// The MatrixInputType is in general automatically deduced
// If (*f) is not overloaded, then FunctionInputType and FunctionOutputType are also
// automatically deduced

// Somehow cannot deduce FunctionInputType and FunctionOutputType if using a lambda
{
	//types::TemplatedEigenMatrix<FunctionOutputType> result(
		//	A.rows(), A.cols());
	Eigen::Matrix<FunctionOutputType, Eigen::Dynamic, Eigen::Dynamic> result(
			A.rows(), A.cols());

	for (size_t i = 0; i < A.rows(); i++)
		for (size_t j = 0; j < A.cols(); j++)
			result(i, j) = (*f)(A(i, j));

	return result;
}

// absolute values component-wise, does not change the matrix type
template<typename MatrixType>
types::TemplatedEigenMatrix<MatrixType> abs(
		const types::EigenExpression<MatrixType>& A)
{
	return fun<typename MatrixType::Scalar, double>(A, std::abs).template cast<
			typename MatrixType::Scalar>();

}

// trace-norm (or Frobenius norm) (CHANGES return type to double)
template<typename MatrixType>
double norm(const types::EigenExpression<MatrixType>& A)
{
	// convert matrix to complex then return its norm
	return (A.template cast<types::cplx>()).norm();
}

// eigenvalues (CHANGES return type to complex)
template<typename MatrixType>
types::cmat evals(const types::EigenExpression<MatrixType>& A)
{
	return (A.template cast<types::cplx>()).eigenvalues();
}

// eigenvectors (CHANGES return type to complex matrix)
template<typename MatrixType>
types::cmat evects(const types::EigenExpression<MatrixType>& A)
{
	Eigen::ComplexEigenSolver<types::cmat> es(A.template cast<types::cplx>());
	return es.eigenvectors();
}

// Kronecker product of 2 matrices, preserve return type
template<typename MatrixType>
types::TemplatedEigenMatrix<MatrixType> kron(
		const types::EigenExpression<MatrixType> &A,
		const types::EigenExpression<MatrixType> &B)
{
	int Acols = A.cols();
	int Arows = A.rows();
	int Bcols = B.cols();
	int Brows = B.rows();

	types::TemplatedEigenMatrix<MatrixType> result;
	result.resize(Arows * Brows, Acols * Bcols);

	for (int i = 0; i < Arows; i++)
		for (int j = 0; j < Acols; j++)
			result.block(i * Brows, j * Bcols, Brows, Bcols) = A(i, j) * B;
	return result;
}

// Kronecker product of a list of matrices, preserve return type
// <MatrixType> is forced to be a matrix by invocation of kron inside the function
template<typename MatrixType>
types::TemplatedEigenMatrix<MatrixType> kron_list(
		const std::vector<MatrixType> &list)
{
	types::TemplatedEigenMatrix<MatrixType> result = list[0];
	for (size_t i = 1; i < list.size(); i++)
		result = kron(result, list[i]);
	return result;
}
// Kronecker product of a matrix with itself $n$ times, preserve return type
template<typename MatrixType>
types::TemplatedEigenMatrix<MatrixType> kron_pow(
		const types::TemplatedEigenMatrix<MatrixType> &A, size_t n)
{
	std::vector<MatrixType> list;
	for (size_t i = 0; i < n; i++)
		list.push_back(A.eval());
	return kron_list(list);
}


// reshape the columns of A and returns a matrix with m rows and n columns
// use column-major order (same as MATLAB)
template<typename MatrixType>
types::TemplatedEigenMatrix<MatrixType> reshape(
		const types::EigenExpression<MatrixType>& A, size_t rows, size_t cols)
{
	size_t rowsA = A.rows();
	size_t colsA = A.cols();

	if (rowsA * colsA != rows * cols)
		throw std::runtime_error("reshape: Dimension mismatch!");

	return Eigen::Map<MatrixType>(static_cast<MatrixType>(A).data(), rows, cols);
}

// permutes the subsystems in a matrix
template<typename MatrixType>
types::TemplatedEigenMatrix<MatrixType> syspermute(
		const types::EigenExpression<MatrixType> &A,
		const std::vector<size_t> perm, const std::vector<size_t> &dims)
{
// Error checks

// check square matrix
	if (!internal::_check_square_mat(A))
		throw std::runtime_error("syspermute: Matrix must be square!");

// check that dims is a valid dimension vector
	if (!internal::_check_dims(dims))
		throw std::runtime_error("syspermute: Invalid dimensions vector!");

// check that dims match the dimension of A
	if (!internal::_check_dims_match_mat(dims, A))
		throw std::runtime_error(
				"syspermute: Dimenisons vector does not match the dimension of the matrix!");

// check that the size of the permutation is OK
	if (!internal::_check_perm(perm, dims))
		throw std::runtime_error("syspermute: Invalid permutation size!");

	size_t dim = static_cast<size_t>(A.rows());
	size_t numdims = dims.size();
	size_t *cdims = new size_t[numdims];
	size_t *cperm = new size_t[numdims];
	MatrixType result(dim, dim);

// copy dims in cdims and perm in cperm
	for (size_t i = 0; i < numdims; i++)
	{
		cdims[i] = dims[i];
		cperm[i] = perm[i];
	}

	size_t iperm = 0;
	size_t jperm = 0;
	for (size_t i = 0; i < dim; i++)
#pragma omp parallel for
		for (size_t j = 0; j < dim; j++)
			internal::_syspermute_worker(numdims, cdims, cperm, i, j, iperm,
					jperm, A, result);

	delete[] cdims;
	delete[] cperm;

	return result; // the permuted matrix
}

// Partial trace over subsystem B in a D_A x D_B system
template<typename MatrixType>
types::TemplatedEigenMatrix<MatrixType> ptrace2(
		const types::EigenExpression<MatrixType> &A,
		const std::vector<size_t> dims)
{
// Error checks
// error checks

// check square matrix
	if (!internal::_check_square_mat(A))
		throw std::runtime_error("ptrace2: Matrix must be square!");

// check dims has only 2 elements
	if (dims.size() != 2)
		throw std::runtime_error("ptrace2: Must have only 2 dimensions!");

// check that dim is a valid dimension vector
	if (!internal::_check_dims(dims))
		throw std::runtime_error("ptrace2: Invalid dimensions vector!");

// check that dims match the dimension of A
	if (!internal::_check_dims_match_mat(dims, A))
		throw std::runtime_error(
				"ptrace2: Dimenisons vector does not match the dimension of the matrix!");

	size_t DA = dims[0];
	size_t DB = dims[1];

	types::TemplatedEigenMatrix<MatrixType> result =
			types::TemplatedEigenMatrix<MatrixType>::Zero(DA, DA);

	for (size_t i = 0; i < DA; i++)
#pragma omp parallel for
		for (size_t j = 0; j < DA; j++)
		{
			result(i, j) = trace(
					static_cast<MatrixType>(A.block(i * DB, j * DB, DB, DB)));
		}
	return result;
}

// partial trace
template<typename MatrixType>
types::TemplatedEigenMatrix<MatrixType> ptrace(
		const types::EigenExpression<MatrixType> &A,
		const std::vector<size_t> &subsys, const std::vector<size_t> &dims)
{
// error checks

// check square matrix
	if (!internal::_check_square_mat(A))
		throw std::runtime_error("ptrace: Matrix must be square!");

// check that dims is a valid dimension vector
	if (!internal::_check_dims(dims))
		throw std::runtime_error("ptrace: Invalid dimensions vector!");

// check that dims match the dimension of A
	if (!internal::_check_dims_match_mat(dims, A))
		throw std::runtime_error(
				"ptrace: Dimenisons vector does not match the dimension of the matrix!");

	if (!internal::_check_subsys(subsys, dims))
		throw std::runtime_error("ptrace: Invalid subsystems!");

	size_t dim = static_cast<size_t>(A.rows());
	size_t numsubsys = subsys.size(); // number of subsystems we trace out
	size_t numdims = dims.size(); // total number of subsystems;
	std::vector<size_t> perm(numdims, 0); // the permutation vector
	std::vector<size_t> permdims; // the permuted dimensions

	types::TemplatedEigenMatrix<MatrixType> result;

// the total dimension of the traced-out subsystems
	size_t dimsubsys = 1;
	for (size_t i = 0; i < numsubsys; i++)
		dimsubsys *= dims[subsys[i]];

	std::vector<size_t> sizeAB;
	sizeAB.push_back(dim / dimsubsys);
	sizeAB.push_back(dimsubsys);

// construct the permutation that bring the traced-out subsystems to the end
	size_t cnt0 = 0;
	size_t cnt1 = 0;
	for (size_t i = 0; i < numdims; i++)
	{
		// we find that i belongs to the subsystem
		if (std::find(subsys.begin(), subsys.end(), i) != subsys.end())
		{
			perm[numdims - numsubsys + cnt0] = i;
			cnt0++;
		}
		else
		{
			perm[cnt1] = i;
			cnt1++;
		}
	}

	return ptrace2(syspermute(A, perm, dims), sizeAB);
}

// partial transpose
template<typename MatrixType>
types::TemplatedEigenMatrix<MatrixType> ptranspose(
		const types::EigenExpression<MatrixType>& A,
		const std::vector<size_t>& subsys, const std::vector<size_t>& dims)
{
// error checks

// check square matrix
	if (!internal::_check_square_mat(A))
		throw std::runtime_error("ptranspose: Matrix must be square!");

// check that dims is a valid dimension vector
	if (!internal::_check_dims(dims))
		throw std::runtime_error("ptranspose: Invalid dimensions vector!");

// check that dims match the dimension of A
	if (!internal::_check_dims_match_mat(dims, A))
		throw std::runtime_error(
				"ptranspose: Dimenisons vector does not match the dimension of the matrix!");

	if (!internal::_check_subsys(subsys, dims))
		throw std::runtime_error("ptranspose: Invalid subsystems!");

	size_t dim = static_cast<size_t>(A.rows());
	size_t numdims = dims.size();
	size_t numsubsys = subsys.size();
	size_t *cdims = new size_t[numdims];
	size_t *midxrow = new size_t[numdims];
	size_t *csubsys = new size_t[numsubsys];

	types::TemplatedEigenMatrix<MatrixType> result = A;

// copy dims in cdims and subsys in csubsys
	for (size_t i = 0; i < numdims; i++)
		cdims[i] = dims[i];
	for (size_t i = 0; i < numsubsys; i++)
		csubsys[i] = subsys[i];

	size_t iperm = 0;
	size_t jperm = 0;
	for (size_t i = 0; i < dim; i++)
	{
		// compute the row multi-index
		internal::_n2multiidx(i, numdims, cdims, midxrow);
#pragma omp parallel for
		for (size_t j = 0; j < dim; j++) // paralelize this code
			internal::_ptranspose_worker(midxrow, numdims, numsubsys, cdims,
					csubsys, i, j, iperm, jperm, A, result);
	}

	delete[] midxrow;
	delete[] cdims;
	delete[] csubsys;

	return result;
}

}

#endif /* UTIL_H_ */
