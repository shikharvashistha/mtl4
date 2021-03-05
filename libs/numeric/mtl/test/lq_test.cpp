// Software License for MTL
//
// Copyright (c) 2007 The Trustees of Indiana University.
//               2008 Dresden University of Technology and the Trustees of Indiana University.
//               2010 SimuNova UG (haftungsbeschränkt), www.simunova.com.
// All rights reserved.
// Authors: Shikhar Vashistha
//
// This file is part of the Matrix Template Library
//
// See also license.mtl.txt in the distribution.

#include <iostream>
#include <boost/utility.hpp>
#include <boost/numeric/mtl/mtl.hpp>


using namespace std;


double f(double) { /* cout << "double\n"; */ return 1.0; }
complex<double> f(complex<double>)
{
    //cout << "complex\n"; 
    return complex<double>(1.0, -1.0);
}


int main(int, char**)
{
    using namespace mtl; using mtl::io::tout;
    unsigned size = 4, row = size + 1, col = size;

    double tol(0.00001);
    dense_vector<double>                vec(size), vec1(size);
    dense2D<double>                     A(row, col), Q(row, row), R(row, col), A_test(row, col),
        A_t(col, row), Q_t(col, col), R_t(col, row), A_t_test(col, row);
    dense2D<complex<double> >                            dz(row, col), Qz(row, row), Rz(row, col);
    dense2D<double, mat::parameters<col_major> >         dc(size, size);
    compressed2D<double>                                 Ac(size, size), Qc(size, size), Rc(size, size), A_testc(size, size);
    A = 0;

    A[0][0] = 1;    A[0][1] = 1;    A[0][2] = 1;
    A[1][0] = 3;    A[1][1] = -1;   A[1][2] = -2;
    A[2][0] = 1;    A[2][1] = 7;    A[2][2] = 1;
    A[3][3] = -10;  A[4][0] = 4;    A[4][2] = 3;
    tout << "A=\n" << A << "\n";
    laplacian_setup(Ac, 2, 2);


    tout << "START-----dense2d---------row > col\n";

    dense2D<double> A1(A[iall][iall]), A2(A);
    boost::tie(Q, R) = lq(A1);
    tout << "R=\n" << R << "\n";
    tout << "Q=\n" << Q << "\n";
    A_test = Q * R - A2;
    tout << "Q*R=\n" << Q * R << "\n";

    tout << "one_norm(Rest A)=" << one_norm(A_test) << "\n";
    MTL_THROW_IF(one_norm(A_test) > tol, mtl::logic_error("wrong LQ decomposition of matrix A"));


    tout << "START------dense2d-------row < col\n";

    A_t = trans(A);
    boost::tie(Q_t, R_t) = lq(A_t);
    tout << "R_t=\n" << R_t << "\n";
    tout << "Q_t=\n" << Q_t << "\n";
    A_t_test = Q_t * R_t - A_t;
    tout << "Q_t*R_t=\n" << Q_t * R_t << "\n";

    tout << "one_norm(Rest A')=" << one_norm(A_t_test) << "\n";
    MTL_THROW_IF(one_norm(A_t_test) > tol, mtl::logic_error("wrong LQ decomposition of matrix trans(A)"));

    tout << "START-------compressed2d-------row > col\n";
#if 1
    boost::tie(Qc, Rc) = lq(Ac);
    tout << "R=\n" << Rc << "\n";
    tout << "Q=\n" << Qc << "\n";
    A_testc = Qc * Rc - Ac;
    tout << "Q*R=\n" << Qc * Rc << "\n";
    tout << "A=\n" << Ac << "\n";

    tout << "one_norm(Rest A)=" << one_norm(A_testc) << "\n";
    MTL_THROW_IF(one_norm(A_testc) > tol, mtl::logic_error("wrong LQ decomposition of matrix A"));
#endif

#if 0
    dz[0][0] = complex<double>(1.0, 0.0);
    dz[0][1] = complex<double>(1.0, 0.0);
    dz[0][2] = complex<double>(1, 0);
    dz[1][0] = complex<double>(1, 0);
    dz[1][1] = complex<double>(-1, 0);
    dz[1][2] = complex<double>(-2, 0);
    dz[2][0] = complex<double>(1, 0);
    dz[2][1] = complex<double>(-2, 0);
    dz[2][2] = complex<double>(1, 0);
    dz[3][3] = complex<double>(-10, 0);
    tout << "MAtrix complex=\n" << dz << "\n";

    tout << "START-----complex---------" << dz[0][0] << "\n";
    //AxB==BxA(For square matrix) LQ==QR(Transpose(A)
    boost::tie(Qz, Rz) = lq(dz);
    // Rz= lq_zerl(dz).second;
    // tout<<"MAtrix  R="<< Rz <<"\n";
    // tout<<"MAtrix  Q="<< Qz <<"\n";
    // tout<<"MAtrix  A=Q*R--outside"<< Qz*Rz <<"\n";
#endif
    /*
    Normal equations
    A^T A x = A^T b
    
    These equations are called the normal equations of the least squares problem
    
    Coefficient matrix  A^TA is the Gram matrix of A

    Equivalent to del f(x) = 0 where f(x) = ||Ax-b||^2

    All solutions of the least squares problem satisfy the normal equations.


    if A has linearly independent columns, then:

    A^T is non-singular
    
    Normal equations have unique solution xcap=(A^T A)^-1 A^T b
     */
    //Rewriting least squares solution using QR/LQ factorization A = QR, QR==LQ(transpose(A)).
    //  x = R^-1 Q^T x b
    /*  1. compute QR factorization A = QR(2mn^2 flops if A is m × n)
        2. matrix - vector product d = Q^T b(2mn flops)
        3. solve Rx = d by back substitution(n^2 flops)
        complexity: 2mn^2 flops
    */
    /*
       Example

         | 3 -6|    |-1 |
       A=| 4 -8|, b=| 7 |
	 | 0  1|    | 2 |
     
    1. QR/LQ factorization A = QR/LQ with

          |3/5 0|    
    	Q=|4/5 0|, R=|5 -10|
     	  | 0  1|    |0	 1 |

    2. calculate d=Q^Tb = (5,2)

    3. solve Rx=d
    		
    	|5 -10| |x1| = |5|
	|0  1 | |x2|   |2|

	solution is x1=5, x2=2	
     
     */
    //Problem with regular method occurs when forming Gram matrix A^T A as it is unstable
    //QR factorization method is more stable because it avoids forming A^T A
    dense2D<double> x(A[iall][iall]),d(A[iall][iall]),b(A[iall][iall]);
    hessian_setup(x, 3.0); hessian_setup(d, 1.0);
    hessian_setup(b, 3.0);
    b[0][0] = -1, b[1][0] = 7, b[2][0] = 2;
    boost::tie(Q, R) = lq(A);
    mult(Q,b,d);
    d = trans(Q) * b;
    //d = trans(Q) * b;
    //lq decomposition is performed here so R==L here 
    x = inv(R) * d;
    tout << "Solution to least square's problem is:" << x;
    return 0;
}
