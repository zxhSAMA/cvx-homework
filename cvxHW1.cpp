#include "pch.h"
#include "gurobi_c++.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "CSVparser.hpp"
//clone from git@github.com:MyBoon/CSVparser.git

/*
	solve the problem of homework 5 question 1
	minimize 0.5 * norm(A * x - b, 2) ^ 2  + mu * norm（x, 1）
	A -> R(m, n)
	b -> R(m)
	mu > 0
	Test Matrix:
	n = 1024
	m = 512
	A = randn(m, n)
	u = sprand(n, 1, 0.1)
	mu = 1e-3
	equivalent model:
	minimize y ^T * y + mu * 1 ^T * s
	subject to y = A * x - b
			   s >= x
			   -s <= x
*/

int main(int argc, char * argv[])
{

	try
	{
		//initial coefficient
		const int n = 1024;
		const int m = 512;
		const float mu = 1.0e-3;
		std::vector<int> one(n);
		for (std::vector<int>::iterator iter = one.begin(); iter != one.end(); iter++) *iter = 1;

		////random initial
		//srand((unsigned int)time(NULL));

		//std::vector<std::vector<double>> A(m);
		//for (std::vector<std::vector<double>>::iterator iter = A.begin(); iter != A.end(); iter++) iter->resize(n);
		//for (std::vector<std::vector<double>>::iterator iter = A.begin(); iter != A.end(); iter++) {
		//	for (std::vector<double>::iterator innerIter = iter->begin(); innerIter != iter->end(); innerIter++) {
		//		*innerIter = rand() % 100;
		//	}
		//}

		//std::vector<double> b(m);
		//for (std::vector<double>::iterator iter = b.begin(); iter != b.end(); iter++) *iter = rand() % 100;

		//initial form csv
		std::vector<std::vector<double>> A(m);
		for (std::vector<std::vector<double>>::iterator iter = A.begin(); iter != A.end(); iter++) iter->resize(n);
		csv::Parser A_csv = csv::Parser("A.csv");
		for (int i = 0; i < n; ++i) {
			A[0][i] = atof(A_csv.getHeaderElement(i).c_str());
		}
		for (int i = 1; i < m; ++i) {
			for (int j = 0; j < n; ++j) {
				A[i][j] = atof(A_csv[i-1][j].c_str());
			}
		}

		std::vector<double> b(m);
		csv::Parser b_csv = csv::Parser("b.csv");
		b[0] = atof(b_csv.getHeaderElement(0).c_str());
		for (int i = 1; i < 512; i++) {
			b[i] = atof(b_csv[i-1][0].c_str());
		}

		//initial environment
		GRBEnv env = GRBEnv();
		GRBModel model = GRBModel(env);

		//initial variables
		GRBVar* x = nullptr;
		GRBVar* y = nullptr;
		GRBVar* s = nullptr;

		x = new GRBVar[n];
		y = new GRBVar[m];
		s = new GRBVar[n];

		for (int i = 0; i < n; ++i) {
			x[i] = model.addVar(0.0 - GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "x_" + std::to_string(i));
		}
		for (int i = 0; i < m; ++i) {
			y[i] = model.addVar(0.0 - GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "y_" + std::to_string(i));
		}
		for (int i = 0; i < n; ++i) {
			s[i] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "s_" + std::to_string(i));
		}

		//set objective function
		GRBQuadExpr func = 0;
		for (int i = 0; i < m; ++i) {
			func += y[i] * y[i];
		}
		for (int i = 0; i < n; ++i) {
			func += mu * one[i] * s[i];
		}
		model.setObjective(func, GRB_MINIMIZE);

		//set constraints
		for (int i = 0; i < m; ++i) {
			GRBLinExpr rhs = 0;
			for (int j = 0; j < n; ++j) {
				rhs += A[i][j] * x[j];
			}
			rhs -= b[i];
			model.addConstr(y[i], GRB_EQUAL, rhs, "constrOFy_" + std::to_string(i));
		}
		for (int j = 0; j < n; ++j) {
			model.addConstr(x[j] <= s[j], "constrOFx<=s_" + std::to_string(j));
			model.addConstr(x[j] >= 0.0 - s[j], "constrOFx>=-s_" + std::to_string(j));
		}

		//optimization
		model.optimize();
		for (int i = 0; i < n; ++i) {
			std::cout << (x + i)->get(GRB_StringAttr_VarName) << "\t" << (x + i)->get(GRB_DoubleAttr_X) << std::endl;
		}

		delete[] x;
		delete[] y;
		delete[] s;
	}
	catch (GRBException e)
	{
		std::cout << "Error Code = " << e.getErrorCode() << std::endl;
		std::cout << "Error Message = " << e.getMessage() << std::endl;
	}
	catch (csv::Error &e)
	{
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "Exception During Optimization" << std::endl;
	}
}
