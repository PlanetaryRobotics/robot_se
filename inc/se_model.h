#ifndef _RL_MODEL_H_
#define _RL_MODEL_H_

typedef enum
{
    x,
    y,
    z,
    roll,
    pitch,
    yaw,
    vx,
    vy,
    vz,
    vroll,
    vpitch,
    vyaw,
    ax,
    ay,
    az
} state ;

#define N_STATE 15 
#define N_MSMT 15

#include "tiny_ekf_struct.h"
#include "tiny_ekf.h"

ekf_t* se_init();
void se_predict(double delta, int idx);
void se_update(double* msmt, int* msmt_up, int num_up, double* msmt_covariance,  int idx);

#endif // _RL_MODEL_H_
