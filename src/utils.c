#include <math.h>

#include "utils.h"

void pol2cart(struct Pol* pol, struct Cart* cart)
{
  cart->x = pol->rho * cos(pol->phi);
  cart->y = pol->rho * sin(pol->phi);
}

void quaternion_from_euler(double roll, double pitch, double yaw, struct Quaternion* quat) 
{
    // Assuming the angles are in radians.
    float c1 = cos(yaw/2);
    float s1 = sin(yaw/2);
    float c2 = cos(pitch/2);
    float s2 = sin(pitch/2);
    float c3 = cos(roll/2);
    float s3 = sin(roll/2);
    float c1c2 = c1*c2;
    float s1s2 = s1*s2;
    quat->w = c1c2*c3 - s1s2*s3;
    quat->x = c1c2*s3 + s1s2*c3;
    quat->y = s1*c2*c3 + c1*s2*s3;
    quat->z = c1*s2*c3 - s1*c2*s3;
}

