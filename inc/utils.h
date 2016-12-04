#ifndef __UTILS__
#define __UTILS__

#include "common.h"

struct Pol
{
  float rho;
  float phi;
};

struct Cart
{
  float x;
  float y;
};



void pol2cart(struct Pol* pol, struct Cart* cart);
void quaternion_from_euler(double roll, double pitch, double yaw, struct Quaternion* quat);

#endif
