/**************************************************************************************************
*                                                                                                 *
* This file is part of HPIPM.                                                                     *
*                                                                                                 *
* HPIPM -- High Performance Interior Point Method.                                                *
* Copyright (C) 2017 by Gianluca Frison.                                                          *
* Developed at IMTEK (University of Freiburg) under the supervision of Moritz Diehl.              *
* All rights reserved.                                                                            *
*                                                                                                 *
* HPMPC is free software; you can redistribute it and/or                                          *
* modify it under the terms of the GNU Lesser General Public                                      *
* License as published by the Free Software Foundation; either                                    *
* version 2.1 of the License, or (at your option) any later version.                              *
*                                                                                                 *
* HPMPC is distributed in the hope that it will be useful,                                        *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                                  *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                                            *
* See the GNU Lesser General Public License for more details.                                     *
*                                                                                                 *
* You should have received a copy of the GNU Lesser General Public                                *
* License along with HPMPC; if not, write to the Free Software                                    *
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA                  *
*                                                                                                 *
* Author: Gianluca Frison, gianluca.frison (at) imtek.uni-freiburg.de                             *
*                                                                                                 *
**************************************************************************************************/



#include <blasfeo_target.h>
#include <blasfeo_common.h>



struct d_dense_qp_dim
	{
	int nv; // number of variables
	int ne; // number of equality constraints
	int nb; // number of box constraints
	int nc; // number of general constraints
	int mem_size; // memory size in bytes
	};

struct d_dense_qp_vec
	{
	struct d_strvec *g; // gradient
	struct d_strvec *be; // dynamics vector
	struct d_strvec *lb; // lower bound
	struct d_strvec *ub; // upper bound
	struct d_strvec *lc; // lower constraint
	struct d_strvec *uc; // upper constraint
	int *idxb; // index of box constraints
	int mem_size; // memory size in bytes
	};

struct d_dense_qp_mat
	{
	struct d_strmat *H; // hessian
	struct d_strmat *A; // dynamics matrix
	struct d_strmat *Ct; // constraints matrix
	int mem_size; // memory size in bytes
	};

struct d_dense_qp
	{
	struct d_dense_qp_dim *dim;
	struct d_dense_qp_vec *vec;
	struct d_dense_qp_mat *mat;
	void (*d_compute_Ctv) (void *qp_dim, void *qp_mat, struct d_strvec *v, struct d_strvec *Ctv); // computes Ct * v
	int mem_size; // memory size in bytes
	};



//
int d_size_dense_qp_dim(struct d_dense_qp_dim *qp_dim);
int d_size_dense_qp_vec(struct d_dense_qp_dim *qp_dim);
int d_size_dense_qp_mat(struct d_dense_qp_dim *qp_dim);
int d_size_dense_qp(struct d_dense_qp_dim *qp_dim);
//
void d_create_dense_qp_dim(struct d_dense_qp_dim *qp_dim, struct d_dense_qp_dim *qp_dim_out, void *memory);
void d_create_dense_qp_vec(struct d_dense_qp_dim *qp_dim, struct d_dense_qp_vec *qp_vec, void *memory);
void d_create_dense_qp_mat(struct d_dense_qp_dim *qp_dim, struct d_dense_qp_mat *qp_mat, void *memory);
void d_create_dense_qp(struct d_dense_qp_dim *qp_dim, struct d_dense_qp *qp, void *memory);
//
void d_init_dense_qp_dim(int nv, int ne, int nb, int nc, struct d_dense_qp_dim *qp_dim_out);
void d_init_dense_qp_vec(struct d_dense_qp_dim *qp_dim, struct d_strvec *g, struct d_strvec *be, struct d_strvec *lb, struct d_strvec *ub, struct d_strvec *lc, struct d_strvec *uc, struct d_dense_qp_vec *qp_vec);
void d_init_dense_qp_mat(struct d_dense_qp_dim *qp_dim, struct d_strmat *H, struct d_strmat *A, struct d_strmat *Ct, struct d_dense_qp_mat *qp_mat);
void d_init_dense_qp(struct d_dense_qp_dim *qp_dim, struct d_strvec *g, struct d_strvec *be, struct d_strvec *lb, struct d_strvec *ub, struct d_strvec *lc, struct d_strvec *uc, struct d_strmat *H, struct d_strmat *A, struct d_strmat *Ct, struct d_dense_qp *qp);
//
void d_cast_dense_qp_dim(int nv, int ne, int nb, int nc, struct d_dense_qp_dim *qp_dim_out);
