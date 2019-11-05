/**************************************************************************************************
*                                                                                                 *
* This file is part of HPIPM.                                                                     *
*                                                                                                 *
* HPIPM -- High-Performance Interior Point Method.                                                *
* Copyright (C) 2019 by Gianluca Frison.                                                          *
* Developed at IMTEK (University of Freiburg) under the supervision of Moritz Diehl.              *
* All rights reserved.                                                                            *
*                                                                                                 *
* The 2-Clause BSD License                                                                        *
*                                                                                                 *
* Redistribution and use in source and binary forms, with or without                              *
* modification, are permitted provided that the following conditions are met:                     *
*                                                                                                 *
* 1. Redistributions of source code must retain the above copyright notice, this                  *
*    list of conditions and the following disclaimer.                                             *
* 2. Redistributions in binary form must reproduce the above copyright notice,                    *
*    this list of conditions and the following disclaimer in the documentation                    *
*    and/or other materials provided with the distribution.                                       *
*                                                                                                 *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND                 *
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED                   *
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE                          *
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR                 *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES                  *
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;                    *
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND                     *
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT                      *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS                   *
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                    *
*                                                                                                 *
* Author: Gianluca Frison, gianluca.frison (at) imtek.uni-freiburg.de                             *
*                                                                                                 *
**************************************************************************************************/



int OCP_QCQP_RES_MEMSIZE(struct OCP_QCQP_DIM *dim)
	{

	// loop index
	int ii;

	// extract ocp qp size
	int N = dim->N;
	int *nx = dim->nx;
	int *nu = dim->nu;
	int *nb = dim->nb;
	int *ng = dim->ng;
	int *nq = dim->nq;
	int *ns = dim->ns;

	// compute core qp size
	int nvt = 0;
	int net = 0;
	int nct = 0;
	for(ii=0; ii<N; ii++)
		{
		nvt += nx[ii]+nu[ii]+2*ns[ii];
		net += nx[ii+1];
		nct += 2*nb[ii]+2*ng[ii]+2*nq[ii]+2*ns[ii];
		}
	nvt += nx[ii]+nu[ii]+2*ns[ii];
	nct += 2*nb[ii]+2*ng[ii]+2*nq[ii]+2*ns[ii];

	int size = 0;

	size += 3*(N+1)*sizeof(struct STRVEC); // res_g res_d res_m
	size += 3*N*sizeof(struct STRVEC); // res_b

	size += 1*SIZE_STRVEC(nvt); // res_g
	size += 1*SIZE_STRVEC(net); // res_b
	size += 2*SIZE_STRVEC(nct); // res_d res_m

	size = (size+63)/64*64; // make multiple of typical cache line size
	size += 1*64; // align once to typical cache line size

	return size;

	}



void OCP_QCQP_RES_CREATE(struct OCP_QCQP_DIM *dim, struct OCP_QCQP_RES *res, void *mem)
	{

	// loop index
	int ii;

	// zero memory (to avoid corrupted memory like e.g. NaN)
	int memsize = OCP_QCQP_RES_MEMSIZE(dim);
	int memsize_m8 = memsize/8; // sizeof(double) is 8
//	int memsize_r8 = memsize - 8*memsize_m8;
	double *double_ptr = mem;
	// XXX exploit that it is multiple of 64 bytes !!!!!
	for(ii=0; ii<memsize_m8-7; ii+=8)
		{
		double_ptr[ii+0] = 0.0;
		double_ptr[ii+1] = 0.0;
		double_ptr[ii+2] = 0.0;
		double_ptr[ii+3] = 0.0;
		double_ptr[ii+4] = 0.0;
		double_ptr[ii+5] = 0.0;
		double_ptr[ii+6] = 0.0;
		double_ptr[ii+7] = 0.0;
		}
//	for(; ii<memsize_m8; ii++)
//		{
//		double_ptr[ii] = 0.0;
//		}
//	char *char_ptr = (char *) (&double_ptr[ii]);
//	for(ii=0; ii<memsize_r8; ii++)
//		{
//		char_ptr[ii] = 0;
//		}

	// extract ocp qp size
	int N = dim->N;
	int *nx = dim->nx;
	int *nu = dim->nu;
	int *nb = dim->nb;
	int *ng = dim->ng;
	int *nq = dim->nq;
	int *ns = dim->ns;

	// compute core qp size
	int nvt = 0;
	int net = 0;
	int nct = 0;
	for(ii=0; ii<N; ii++)
		{
		nvt += nx[ii]+nu[ii]+2*ns[ii];
		net += nx[ii+1];
		nct += 2*nb[ii]+2*ng[ii]+2*nq[ii]+2*ns[ii];
		}
	nvt += nx[ii]+nu[ii]+2*ns[ii];
	nct += 2*nb[ii]+2*ng[ii]+2*nq[ii]+2*ns[ii];


	// vector struct
	struct STRVEC *sv_ptr = (struct STRVEC *) mem;

	res->res_g = sv_ptr;
	sv_ptr += N+1;
	res->res_b = sv_ptr;
	sv_ptr += N;
	res->res_d = sv_ptr;
	sv_ptr += N+1;
	res->res_m = sv_ptr;
	sv_ptr += N+1;


	// align to typicl cache line size
	size_t s_ptr = (size_t) sv_ptr;
	s_ptr = (s_ptr+63)/64*64;


	// void stuf
	char *c_ptr = (char *) s_ptr;

	CREATE_STRVEC(nvt, res->res_g, c_ptr);
	c_ptr += SIZE_STRVEC(nvt);

	CREATE_STRVEC(net, res->res_b, c_ptr);
	c_ptr += SIZE_STRVEC(net);

	CREATE_STRVEC(nct, res->res_d, c_ptr);
	c_ptr += SIZE_STRVEC(nct);

	CREATE_STRVEC(nct, res->res_m, c_ptr);
	c_ptr += SIZE_STRVEC(nct);

	// alias
	//
	c_ptr = (char *) res->res_g->pa;
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(nu[ii]+nx[ii]+2*ns[ii], res->res_g+ii, c_ptr);
		c_ptr += nu[ii]*sizeof(REAL);
		c_ptr += nx[ii]*sizeof(REAL);
		c_ptr += ns[ii]*sizeof(REAL);
		c_ptr += ns[ii]*sizeof(REAL);
		}
	//
	c_ptr = (char *) res->res_b->pa;
	for(ii=0; ii<N; ii++)
		{
		CREATE_STRVEC(nx[ii+1], res->res_b+ii, c_ptr);
		c_ptr += (nx[ii+1])*sizeof(REAL);
		}
	//
	c_ptr = (char *) res->res_d->pa;
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(2*nb[ii]+2*ng[ii]+2*nq[ii]+2*ns[ii], res->res_d+ii, c_ptr);
		c_ptr += nb[ii]*sizeof(REAL);
		c_ptr += ng[ii]*sizeof(REAL);
		c_ptr += nq[ii]*sizeof(REAL);
		c_ptr += nb[ii]*sizeof(REAL);
		c_ptr += ng[ii]*sizeof(REAL);
		c_ptr += nq[ii]*sizeof(REAL);
		c_ptr += ns[ii]*sizeof(REAL);
		c_ptr += ns[ii]*sizeof(REAL);
		}
	//
	c_ptr = (char *) res->res_m->pa;
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(2*nb[ii]+2*ng[ii]+2*nq[ii]+2*ns[ii], res->res_m+ii, c_ptr);
		c_ptr += nb[ii]*sizeof(REAL);
		c_ptr += ng[ii]*sizeof(REAL);
		c_ptr += nq[ii]*sizeof(REAL);
		c_ptr += nb[ii]*sizeof(REAL);
		c_ptr += ng[ii]*sizeof(REAL);
		c_ptr += nq[ii]*sizeof(REAL);
		c_ptr += ns[ii]*sizeof(REAL);
		c_ptr += ns[ii]*sizeof(REAL);
		}



	res->dim = dim;

	res->memsize = OCP_QCQP_RES_MEMSIZE(dim);


#if defined(RUNTIME_CHECKS)
	if(c_ptr > ((char *) mem) + res->memsize)
		{
		printf("\ncreate_ocp_qp_res: outside memory bounds!\n\n");
		exit(1);
		}
#endif


	return;

	}



int OCP_QCQP_RES_WS_MEMSIZE(struct OCP_QCQP_DIM *dim)
	{

	// loop index
	int ii;

	// extract ocp qp size
	int N = dim->N;
	int *nx = dim->nx;
	int *nu = dim->nu;
	int *nb = dim->nb;
	int *ng = dim->ng;
	int *nq = dim->nq;
	int *ns = dim->ns;

	// compute core qp size and max size
	int nuM = 0;
	int nxM = 0;
	int nbM = 0;
	int ngM = 0;
	int nqM = 0;
	int nsM = 0;
	for(ii=0; ii<=N; ii++)
		{
		nuM = nu[ii]>nuM ? nu[ii] : nuM;
		nxM = nx[ii]>nxM ? nx[ii] : nxM;
		nbM = nb[ii]>nbM ? nb[ii] : nbM;
		ngM = ng[ii]>ngM ? ng[ii] : ngM;
		nqM = nq[ii]>nqM ? nq[ii] : nqM;
		nsM = ns[ii]>nsM ? ns[ii] : nsM;
		}

	int size = 0;

	size += (4+2*(N+1))*sizeof(struct STRVEC); // tmp_nuxM 2*tmp_nbgqM tmp_nsM q_fun q_adj

	size += 1*SIZE_STRVEC(nuM+nxM); // tmp_nuxM q_adj
	size += 2*SIZE_STRVEC(nbM+ngM+nqM); // tmp_nbgqM
	size += 1*SIZE_STRVEC(nsM); // tmp_nsM
	for(ii=0; ii<=N; ii++)
		{
		size += 1*SIZE_STRVEC(nq[ii]); // q_fun
		size += 1*SIZE_STRVEC(nu[ii]+nx[ii]); // q_adj
		}

	size = (size+63)/64*64; // make multiple of typical cache line size
	size += 1*64; // align once to typical cache line size

	return size;

	}



void OCP_QCQP_RES_WS_CREATE(struct OCP_QCQP_DIM *dim, struct OCP_QCQP_RES_WS *ws, void *mem)
	{

	// loop index
	int ii;

	// zero memory (to avoid corrupted memory like e.g. NaN)
	int memsize = OCP_QCQP_RES_WS_MEMSIZE(dim);
	int memsize_m8 = memsize/8; // sizeof(double) is 8
//	int memsize_r8 = memsize - 8*memsize_m8;
	double *double_ptr = mem;
	// XXX exploit that it is multiple of 64 bytes !!!!!
	for(ii=0; ii<memsize_m8-7; ii+=8)
		{
		double_ptr[ii+0] = 0.0;
		double_ptr[ii+1] = 0.0;
		double_ptr[ii+2] = 0.0;
		double_ptr[ii+3] = 0.0;
		double_ptr[ii+4] = 0.0;
		double_ptr[ii+5] = 0.0;
		double_ptr[ii+6] = 0.0;
		double_ptr[ii+7] = 0.0;
		}
//	for(; ii<memsize_m8; ii++)
//		{
//		double_ptr[ii] = 0.0;
//		}
//	char *char_ptr = (char *) (&double_ptr[ii]);
//	for(ii=0; ii<memsize_r8; ii++)
//		{
//		char_ptr[ii] = 0;
//		}

	// extract ocp qp size
	int N = dim->N;
	int *nx = dim->nx;
	int *nu = dim->nu;
	int *nb = dim->nb;
	int *ng = dim->ng;
	int *nq = dim->nq;
	int *ns = dim->ns;


	// compute core qp size and max size
	int nuM = 0;
	int nxM = 0;
	int nbM = 0;
	int ngM = 0;
	int nqM = 0;
	int nsM = 0;
	for(ii=0; ii<=N; ii++)
		{
		nuM = nu[ii]>nuM ? nu[ii] : nuM;
		nxM = nx[ii]>nxM ? nx[ii] : nxM;
		nbM = nb[ii]>nbM ? nb[ii] : nbM;
		ngM = ng[ii]>ngM ? ng[ii] : ngM;
		nqM = nq[ii]>nqM ? nq[ii] : nqM;
		nsM = ns[ii]>nsM ? ns[ii] : nsM;
		}


	// vector struct
	struct STRVEC *sv_ptr = (struct STRVEC *) mem;

	ws->tmp_nuxM = sv_ptr;
	sv_ptr += 1;
	ws->tmp_nbgqM = sv_ptr;
	sv_ptr += 2;
	ws->tmp_nsM = sv_ptr;
	sv_ptr += 1;
	ws->q_fun = sv_ptr;
	sv_ptr += N+1;
	ws->q_adj = sv_ptr;
	sv_ptr += N+1;


	// align to typicl cache line size
	size_t s_ptr = (size_t) sv_ptr;
	s_ptr = (s_ptr+63)/64*64;


	// void stuf
	char *c_ptr = (char *) s_ptr;


	CREATE_STRVEC(nuM+nxM, ws->tmp_nuxM, c_ptr);
	c_ptr += (ws->tmp_nuxM)->memsize;

	CREATE_STRVEC(nbM+ngM, ws->tmp_nbgqM+0, c_ptr);
	c_ptr += (ws->tmp_nbgqM+0)->memsize;

	CREATE_STRVEC(nbM+ngM, ws->tmp_nbgqM+1, c_ptr);
	c_ptr += (ws->tmp_nbgqM+1)->memsize;

	CREATE_STRVEC(nsM, ws->tmp_nsM, c_ptr);
	c_ptr += (ws->tmp_nsM)->memsize;

	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(nq[ii], ws->q_fun+ii, c_ptr);
		c_ptr += (ws->q_fun+ii)->memsize;
		}

	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(nu[ii]+nx[ii], ws->q_adj+ii, c_ptr);
		c_ptr += (ws->q_adj+ii)->memsize;
		}


	ws->memsize = OCP_QCQP_RES_WS_MEMSIZE(dim);


#if defined(RUNTIME_CHECKS)
	if(c_ptr > ((char *) mem) + ws->memsize)
		{
		printf("\ncreate_ocp_qp_res_workspace: outside memory bounds!\n\n");
		exit(1);
		}
#endif


	return;

	}



void OCP_QCQP_RES_COMPUTE(struct OCP_QCQP *qp, struct OCP_QCQP_SOL *qp_sol, struct OCP_QCQP_RES *res, struct OCP_QCQP_RES_WS *ws)
	{

	// loop index
	int ii, jj;

	//
	int N = qp->dim->N;
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;
	int *nb = qp->dim->nb;
	int *ng = qp->dim->ng;
	int *nq = qp->dim->nq;
	int *ns = qp->dim->ns;

	int nct = 0;
	for(ii=0; ii<=N; ii++)
		nct += 2*nb[ii]+2*ng[ii]+2*ns[ii];

	REAL nct_inv = 1.0/nct;

	struct STRMAT *BAbt = qp->BAbt;
	struct STRMAT *RSQrq = qp->RSQrq;
	struct STRMAT *DCt = qp->DCt;
	struct STRVEC *b = qp->b;
	struct STRVEC *rqz = qp->rqz;
	struct STRVEC *d = qp->d;
	struct STRVEC *m = qp->m;
	int **idxb = qp->idxb;
	struct STRVEC *Z = qp->Z;
	int **idxs = qp->idxs;
	struct STRMAT **Hq = qp->Hq;
	struct STRVEC **gq = qp->gq;

	struct STRVEC *ux = qp_sol->ux;
	struct STRVEC *pi = qp_sol->pi;
	struct STRVEC *lam = qp_sol->lam;
	struct STRVEC *t = qp_sol->t;

	struct STRVEC *res_g = res->res_g;
	struct STRVEC *res_b = res->res_b;
	struct STRVEC *res_d = res->res_d;
	struct STRVEC *res_m = res->res_m;

	struct STRVEC *tmp_nuxM = ws->tmp_nuxM;
	struct STRVEC *tmp_nbgqM = ws->tmp_nbgqM;
	struct STRVEC *tmp_nsM = ws->tmp_nsM;

	struct STRVEC *q_fun = ws->q_fun;
	struct STRVEC *q_adj = ws->q_adj;

	int nx0, nx1, nu0, nu1, nb0, ng0, nq0, ns0;

	//
	REAL tmp;
	REAL mu = 0.0;

	// loop over stages
	for(ii=0; ii<=N; ii++)
		{

		nx0 = nx[ii];
		nu0 = nu[ii];
		nb0 = nb[ii];
		ng0 = ng[ii];
		nq0 = nq[ii];
		ns0 = ns[ii];

		SYMV_L(nu0+nx0, nu0+nx0, 1.0, RSQrq+ii, 0, 0, ux+ii, 0, 1.0, rqz+ii, 0, res_g+ii, 0);

		if(ii>0)
			AXPY(nx0, -1.0, pi+(ii-1), 0, res_g+ii, nu0, res_g+ii, nu0);

		if(nb0+ng0+nq0>0)
			{
			AXPY(nb0+ng0, -1.0, lam+ii, 0, lam+ii, nb0+ng0+nq0, tmp_nbgqM+0, 0);
//			AXPY(nb0+ng0,  1.0, d+ii, 0, t+ii, 0, res_d+ii, 0);
//			AXPY(nb0+ng0,  1.0, d+ii, nb0+ng0, t+ii, nb0+ng0, res_d+ii, nb0+ng0);
			AXPY(2*nb0+2*ng0+2*nq0,  1.0, d+ii, 0, t+ii, 0, res_d+ii, 0);
			// box
			if(nb0>0)
				{
				VECAD_SP(nb0, 1.0, tmp_nbgqM+0, 0, idxb[ii], res_g+ii, 0);
				VECEX_SP(nb0, 1.0, idxb[ii], ux+ii, 0, tmp_nbgqM+1, 0);
				}
			// general
			if(ng0>0)
				{
				GEMV_NT(nu0+nx0, ng0, 1.0, 1.0, DCt+ii, 0, 0, tmp_nbgqM+0, nb[ii], ux+ii, 0, 1.0, 0.0, res_g+ii, 0, tmp_nbgqM+1, nb0, res_g+ii, 0, tmp_nbgqM+1, nb0);
				}
			// quadratic
			if(nq0>0)
				{
//				AXPY(nq0,  1.0, d, 2*nb0+2*ng0+2*ns, t, 2*nb+2*ng+2*ns, res_d, 2*nb+2*ng+2*ns);
				if(ws->use_q_fun & ws->use_q_adj)
					{
					VECCP(nq0, ws->q_fun+ii, 0, tmp_nbgqM+1, nb0+ng0);
					AXPY(nu0+nx0, 1.0, ws->q_adj+ii, 0, res_g+ii, 0, res_g+ii, 0);
					}
				else
					{
					for(jj=0; jj<nq0; jj++)
						{
						SYMV_L(nu0+nx0, nu0+nx0, 1.0, &Hq[ii][jj], 0, 0, ux+ii, 0, 0.0, tmp_nuxM, 0, tmp_nuxM, 0);
#ifdef DOUBLE_PRECISION
						tmp = BLASFEO_DVECEL(tmp_nbgqM+0, nb0+ng0+jj);
#else
						tmp = BLASFEO_SVECEL(tmp_nbgqM+0, nb0+ng0+jj);
#endif
						AXPY(nu0+nx0, tmp, tmp_nuxM, 0, res_g+ii, 0, res_g+ii, 0);
						AXPY(nu0+nx0, tmp, &gq[ii][jj], 0, res_g+ii, 0, res_g+ii, 0);
						AXPY(nu0+nx0, 0.5, tmp_nuxM, 0, &gq[ii][jj], 0, tmp_nuxM, 0);
						tmp = DOT(nu0+nx0, tmp_nuxM, 0, ux+ii, 0);
#ifdef DOUBLE_PRECISION
						BLASFEO_DVECEL(tmp_nbgqM+1, nb0+ng0+jj) = tmp;
#else
						BLASFEO_SVECEL(tmp_nbgqM+1, nb0+ng0+jj) = tmp;
#endif
						}
					}
				}
			AXPY(nb0+ng0+nq0, -1.0, tmp_nbgqM+1, 0, res_d+ii, 0, res_d+ii, 0);
			AXPY(nb0+ng0+nq0,  1.0, tmp_nbgqM+1, 0, res_d+ii, nb0+ng0+nq0, res_d+ii, nb0+ng0+nq0);
			}
		if(ns0>0)
			{
			// res_g
			GEMV_DIAG(2*ns0, 1.0, Z+ii, 0, ux+ii, nu0+nx0, 1.0, rqz+ii, nu0+nx0, res_g+ii, nu0+nx0);
			AXPY(2*ns0, -1.0, lam+ii, 2*nb0+2*ng0+2*nq0, res_g+ii, nu0+nx0, res_g+ii, nu0+nx0);
			VECEX_SP(ns0, 1.0, idxs[ii], lam+ii, 0, tmp_nsM, 0);
			AXPY(ns0, -1.0, tmp_nsM, 0, res_g+ii, nu0+nx0, res_g+ii, nu0+nx0);
			VECEX_SP(ns0, 1.0, idxs[ii], lam+ii, nb0+ng0+nq0, tmp_nsM, 0);
			AXPY(ns0, -1.0, tmp_nsM, 0, res_g+ii, nu0+nx0+ns0, res_g+ii, nu0+nx0+ns0);
			// res_d
			VECAD_SP(ns0, -1.0, ux+ii, nu0+nx0, idxs[ii], res_d+ii, 0);
			VECAD_SP(ns0, -1.0, ux+ii, nu0+nx0+ns0, idxs[ii], res_d+ii, nb0+ng0+nq0);
			AXPY(2*ns0, -1.0, ux+ii, nu0+nx0, t+ii, 2*nb0+2*ng0+2*nq0, res_d+ii, 2*nb0+2*ng0+2*nq0);
			AXPY(2*ns0, 1.0, d+ii, 2*nb0+2*ng0+2*nq0, res_d+ii, 2*nb0+2*ng0+2*nq0, res_d+ii, 2*nb0+2*ng0+2*nq0);
			}

		if(ii<N)
			{

			nu1 = nu[ii+1];
			nx1 = nx[ii+1];

			AXPY(nx1, -1.0, ux+(ii+1), nu1, b+ii, 0, res_b+ii, 0);

			GEMV_NT(nu0+nx0, nx1, 1.0, 1.0, BAbt+ii, 0, 0, pi+ii, 0, ux+ii, 0, 1.0, 1.0, res_g+ii, 0, res_b+ii, 0, res_g+ii, 0, res_b+ii, 0);

			}

		mu += VECMULDOT(2*nb0+2*ng0+2*nq0+2*ns0, lam+ii, 0, t+ii, 0, res_m+ii, 0);
		AXPY(2*nb0+2*ng0+2*nq0+2*ns0, -1.0, m+ii, 0, res_m+ii, 0, res_m+ii, 0);

		}

	res->res_mu = mu*nct_inv;

	return;

	}



void OCP_QCQP_RES_COMPUTE_INF_NORM(struct OCP_QCQP_RES *res)
	{

	struct OCP_QCQP_DIM *dim = res->dim;
	int N = dim->N;
	int *nx = dim->nx;
	int *nu = dim->nu;
	int *nb = dim->nb;
	int *ng = dim->ng;
	int *nq = dim->nq;
	int *ns = dim->ns;

	int ii;

	int nv = 0;
	int ne = 0;
	int nc = 0;

	for(ii=0; ii<N; ii++)
		{
		nv += nu[ii]+nx[ii]+2*ns[ii];
		ne += nx[ii+1];
		nc += 2*nb[ii]+2*ng[ii]+2*nq[ii]+2*ns[ii];
		}
	ii = N;
	nv += nu[ii]+nx[ii]+2*ns[ii];
	nc += 2*nb[ii]+2*ng[ii]+2*nq[ii]+2*ns[ii];

	// compute infinity norm
	VECNRM_INF(nv, res->res_g, 0, res->res_max+0);
	VECNRM_INF(ne, res->res_b, 0, res->res_max+1);
	VECNRM_INF(nc, res->res_d, 0, res->res_max+2);
	VECNRM_INF(nc, res->res_m, 0, res->res_max+3);

	return;

	}



void OCP_QCQP_RES_GET_MAX_RES_STAT(struct OCP_QCQP_RES *res, REAL *value)
	{

	*value = res->res_max[0];

	return;

	}



void OCP_QCQP_RES_GET_MAX_RES_EQ(struct OCP_QCQP_RES *res, REAL *value)
	{

	*value = res->res_max[1];

	return;

	}



void OCP_QCQP_RES_GET_MAX_RES_INEQ(struct OCP_QCQP_RES *res, REAL *value)
	{

	*value = res->res_max[2];

	return;

	}



void OCP_QCQP_RES_GET_MAX_RES_COMP(struct OCP_QCQP_RES *res, REAL *value)
	{

	*value = res->res_max[3];

	return;

	}

