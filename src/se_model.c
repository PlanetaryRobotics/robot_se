#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "vecmat_utils.h"
#include "se_model.h"

void se_init(ekf_t* p_ekf)
{
    if (p_ekf == NULL)
        return;

    ekf_init(p_ekf, N, M);
    
    int i, j;

    for (i=0; i<N; ++i)
        p_ekf->x[i] = 0;

    for (i=0; i<N; ++i)
        for (j=0; i<N; ++i)
            p_ekf->F[i][j] = 0;

    for (i=0; i<N; ++i)
        p_ekf->A[i][i] = 1;
    
    for (i = 0 ; i < M ; i++)
        p_ekf->C[i][i] = 1;     

    for (i=0; i<N; ++i)
        for (j=0; i<N; ++i)
            p_ekf->Q[i][j] = 0;

    p_ekf->Q[0][0] = .05;
    p_ekf->Q[1][1] = .05;
    p_ekf->Q[2][2] = .06;
    p_ekf->Q[3][3] = .03;
    p_ekf->Q[4][4] = .03;
    p_ekf->Q[5][5] = .06;
    p_ekf->Q[6][6] = .025;
    p_ekf->Q[7][7] = .025;
    p_ekf->Q[8][8] = .04;
    p_ekf->Q[9][9] = .01;
    p_ekf->Q[10][10] = .01;
    p_ekf->Q[11][11] = .02;
    p_ekf->Q[12][12] = .01;
    p_ekf->Q[13][13] = .01;
    p_ekf->Q[14][14] = .015;

    for (i=0; i<N; ++i)
        p_ekf->P[i][i] = 1e-9;

    for (i=0; i<M; ++i)
        p_ekf->hx[i] = 0;

    for (i=0; i<M; ++i)
        p_ekf->R[i][i] = .01;

    for (i=0; i<N; ++i)
        p_ekf->H[i][i] = 1;      

}

const double PI = 3.1415926;
const double TAU = 6.28318;

static double wrapRotation(double rotation)
{
    if (rotation > PI)
    {
        int q = (int)((rotation - PI) / TAU);
        return rotation - (q+1)*TAU;
    }
    else if (rotation < -PI)
    {
        int q = (int)((PI - rotation) / TAU);
        return rotation + (q+1)*TAU;
    }
    else 
    {
        return rotation;
    }
}

static void constrainStateAngles(ekf_t* p_ekf)
{
    p_ekf->x[roll] = wrapRotation(p_ekf->x[roll]);
    p_ekf->x[pitch] = wrapRotation(p_ekf->x[pitch]);
    p_ekf->x[yaw] = wrapRotation(p_ekf->x[yaw]);
}

void se_predict(double delta, ekf_t* p_ekf)
{
    if (p_ekf == NULL)
        return;

    double sp = sin(p_ekf->x[pitch]);
    double cp = cos(p_ekf->x[pitch]);
    double sr = sin(p_ekf->x[roll]);
    double cr = cos(p_ekf->x[roll]);
    double sy = sin(p_ekf->x[yaw]);
    double cy = cos(p_ekf->x[yaw]);
    
    p_ekf->A[x][vx] = cy * cp * delta;
    p_ekf->A[x][vy] = (cy * sp * sr - sy * cr) * delta;
    p_ekf->A[x][vz] = (cy * sp * cr + sy * sr) * delta;

    p_ekf->A[x][ax] = 0.5 * p_ekf->A[x][vx] * delta;
    p_ekf->A[x][ay] = 0.5 * p_ekf->A[x][vy] * delta;
    p_ekf->A[x][az] = 0.5 * p_ekf->A[x][vz] * delta;

    p_ekf->A[y][vx] = sy * cp * delta;
    p_ekf->A[y][vy] = (sy * sp * sr + cy * cr) * delta;
    p_ekf->A[y][vz] = (sy * sp * cr - cy * sr) * delta;

    p_ekf->A[y][ax] = 0.5 * p_ekf->A[y][vx] * delta;
    p_ekf->A[y][ay] = 0.5 * p_ekf->A[y][vy] * delta;
    p_ekf->A[y][az] = 0.5 * p_ekf->A[y][vz] * delta;
    
    p_ekf->A[z][vx] = -sp * delta;
    p_ekf->A[z][vy] = cp * sr * delta;
    p_ekf->A[z][vz] = cp * cr * delta;

    p_ekf->A[z][ax] = 0.5 * p_ekf->A[z][vx] * delta;
    p_ekf->A[z][ay] = 0.5 * p_ekf->A[z][vy] * delta;
    p_ekf->A[z][az] = 0.5 * p_ekf->A[z][vz] * delta;

    p_ekf->A[roll][vroll] = p_ekf->A[x][vx];
    p_ekf->A[roll][vpitch] = p_ekf->A[x][vy];
    p_ekf->A[roll][vyaw] = p_ekf->A[x][vz];

    p_ekf->A[pitch][vroll] = p_ekf->A[y][vx];
    p_ekf->A[pitch][vpitch] = p_ekf->A[y][vy];
    p_ekf->A[pitch][vyaw] = p_ekf->A[y][vz];
    
    p_ekf->A[yaw][vroll] = p_ekf->A[z][vx];
    p_ekf->A[yaw][vpitch] = p_ekf->A[z][vy];
    p_ekf->A[yaw][vyaw] = p_ekf->A[z][vz];

    p_ekf->A[vx][ax] = delta;
    p_ekf->A[vy][ay] = delta;
    p_ekf->A[vz][az] = delta;
    
    double xcoeff = 0.0;
    double ycoeff = 0.0;
    double zcoeff = 0.0;
    double sqrby2 = delta * delta * 0.5;

    ycoeff = cy * sp * cr + sy * sr;
    zcoeff = -cy * sp * sr + sy * cr;
    double dFx_dR = (ycoeff * p_ekf->x[vy] + zcoeff * p_ekf->x[vz]) * delta + (ycoeff * p_ekf->x[ay] + zcoeff * p_ekf->x[az]) * sqrby2;
    double dFR_dR = 1 + (ycoeff * p_ekf->x[vpitch] + zcoeff * p_ekf->x[vyaw]) * delta;

    xcoeff = -cy * sp;
    ycoeff = cy * cp * sr;
    zcoeff = cy * cp * cr;
    double dFx_dP = (xcoeff * p_ekf->x[vx] + ycoeff * p_ekf->x[vy] + zcoeff * p_ekf->x[vz]) * delta +
                    (xcoeff * p_ekf->x[ax] + ycoeff * p_ekf->x[ay] + zcoeff * p_ekf->x[az]) * sqrby2;
    double dFR_dP = (xcoeff * p_ekf->x[vroll] + ycoeff * p_ekf->x[pitch] + zcoeff * p_ekf->x[vyaw]) * delta;

    xcoeff = -sy * cp;
    ycoeff = -sy * sp * sr - cy * cr;
    zcoeff = -sy * sp * cr + cy * sr;
    double dFx_dY = (xcoeff * p_ekf->x[vx] + ycoeff * p_ekf->x[vy] + zcoeff * p_ekf->x[vz]) * delta +
                    (xcoeff * p_ekf->x[ax] + ycoeff * p_ekf->x[ay] + zcoeff * p_ekf->x[az]) * sqrby2;
    double dFR_dY = (xcoeff * p_ekf->x[vroll] + ycoeff * p_ekf->x[vpitch] + zcoeff * p_ekf->x[vyaw]) * delta;
    
    ycoeff = sy * sp * cr - cy * sr;
    zcoeff = -sy * sp * sr - cy * cr;
    double dFy_dR = (ycoeff * p_ekf->x[vy] + zcoeff * p_ekf->x[vz]) * delta +
                    (ycoeff * p_ekf->x[ay] + zcoeff * p_ekf->x[az]) * sqrby2;
    double dFP_dR = (ycoeff * p_ekf->x[vpitch] + zcoeff * p_ekf->x[vyaw]) * delta;

    xcoeff = -sy * sp;
    ycoeff = sy * cp * sr;
    zcoeff = sy * cp * cr;
    
    double dFy_dP = (xcoeff * p_ekf->x[vx] + ycoeff * p_ekf->x[vy] + zcoeff * p_ekf->x[vz]) * delta +
                    (xcoeff * p_ekf->x[ax] + ycoeff * p_ekf->x[ay] + zcoeff * p_ekf->x[az]) * sqrby2;
    double dFP_dP = 1 + (xcoeff * p_ekf->x[vroll] + ycoeff * p_ekf->x[vpitch] + zcoeff * p_ekf->x[vyaw]) * delta;

    xcoeff = cy * cp;
    ycoeff = cy * sp * sr - sy * cr;
    zcoeff = cy * sp * cr + sy * sr;
    double dFy_dY = (xcoeff * p_ekf->x[vx] + ycoeff * p_ekf->x[vy] + zcoeff * p_ekf->x[vz]) * delta +
                    (xcoeff * p_ekf->x[ax] + ycoeff * p_ekf->x[ay] + zcoeff * p_ekf->x[az]) * sqrby2;
    double dFP_dY = (xcoeff * p_ekf->x[vroll] + ycoeff * p_ekf->x[vpitch] + zcoeff * p_ekf->x[vyaw]) * delta;

    ycoeff = cp * cr;
    zcoeff = -cp * sr;
    double dFz_dR = (ycoeff * p_ekf->x[vy] + zcoeff * p_ekf->x[vz]) * delta +
                    (ycoeff * p_ekf->x[ay] + zcoeff * p_ekf->x[az]) * sqrby2;
    double dFY_dR = (ycoeff * p_ekf->x[vpitch] + zcoeff * p_ekf->x[vyaw]) * delta;

    xcoeff = -cp;
    ycoeff = -sp * sr;
    zcoeff = -sp * cr;
    double dFz_dP = (xcoeff * p_ekf->x[vx] + ycoeff * p_ekf->x[vy] + zcoeff * p_ekf->x[vz]) * delta +
                    (xcoeff * p_ekf->x[ax] + ycoeff * p_ekf->x[ay] + zcoeff * p_ekf->x[az]) * sqrby2;
    double dFY_dP = (xcoeff * p_ekf->x[vroll] + ycoeff * p_ekf->x[vpitch] + zcoeff * p_ekf->x[vyaw]) * delta;

    int i,j;
    for (i=0 ; i<N ; i++)
        for (j=0 ; j<N ; j++)
            p_ekf->F[i][j] = p_ekf->A[i][j];
    	     
    p_ekf->F[x][roll] = dFx_dR;
    p_ekf->F[x][pitch] = dFx_dP;
    p_ekf->F[x][yaw] = dFx_dY;
    p_ekf->F[y][roll] = dFy_dR;
    p_ekf->F[y][pitch] = dFy_dP;
    p_ekf->F[y][yaw] = dFy_dY;
    p_ekf->F[z][roll] = dFz_dR;
    p_ekf->F[z][pitch] = dFz_dP;
    p_ekf->F[roll][roll] = dFR_dR;
    p_ekf->F[roll][pitch] = dFR_dP;
    p_ekf->F[roll][yaw] = dFR_dY;
    p_ekf->F[pitch][roll] = dFP_dR;
    p_ekf->F[pitch][pitch] = dFP_dP;
    p_ekf->F[pitch][yaw] = dFP_dY;
    p_ekf->F[yaw][roll] = dFY_dR;
    p_ekf->F[yaw][pitch] = dFY_dP;
        
    ekf_predict(p_ekf, delta);    

    p_ekf->fx[roll] = wrapRotation(p_ekf->fx[roll]);
    p_ekf->fx[pitch] = wrapRotation(p_ekf->fx[pitch]);
    p_ekf->fx[yaw] = wrapRotation(p_ekf->fx[yaw]);

    //TODO Apply control here. KR
}

void se_update(double* msmt, int* msmt_up, int num_up,  double* msmt_covariance, ekf_t* p_ekf, int idx)
{
    if (p_ekf == NULL)
        return;

    int i;

    // Measurements are already in the form of state variables. So C is just unity
    for (i = 0 ; i < M ; i++)
        p_ekf->C[i][i] = 1;     

    // msmt
    double z[M];
    for (i = 0 ; i < M ; i++)
        z[i] = msmt[i];     
  
    // H , msmt jacobian            
    for (i = 0 ; i < M ; i++)
    {
        int j;
        for (j = 0 ; j < N ; j++)
	{   // TODO  what if z is indeed 0 ??? needs fix. KR
	    p_ekf->H[i][j] = 0;
	}
    }

    // H , msmt jacobian            
    for (i = 0 ; i < num_up ; i++)
    { 
        int idx = msmt_up[i];
        p_ekf->H[idx][idx] = 1;
    }

    // msmt covariance
    int cnt = 0;
    for (i = 0 ; i < M ; i++)
    {
        int j;
        for (j = 0 ; j < N ; j++)
	{
  	    p_ekf->R[i][j] = msmt_covariance[cnt++];
            if ((i == j) && (p_ekf->R[i][j] < 0))    // Variance negative
	        p_ekf->R[i][j] = abs(p_ekf->R[i][j]);            
            if ((i == j) && (p_ekf->R[i][j] < 1e-9)) // Variance too small
  	        p_ekf->R[i][j] = 1e-9;
	}
    }

    // update
    ekf_step(p_ekf, z, idx);

    // constraint angles
    constrainStateAngles(p_ekf);    
}

/*int tokenize_l(char str[], double out[])
{  
    const char delim[2] = ",";            
    int num_tokens = 0;
    
    char* token = strtok(str, delim);       
    while(token != NULL) 
    {
        out[num_tokens++] = atof(token);
        token = strtok(NULL, delim); 
    }
    return num_tokens;
 }*/





