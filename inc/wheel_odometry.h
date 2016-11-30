#ifndef __WHEEL_ODOMETRY__
#define __WHEEL_ODOMETRY__

#define WHEEL_BASE_LENGTH .57
#define CLICKS_PER_METER -1425

struct WheelOdometry
{
  struct Odometry odom;
  float x;
  float y;
  float theta;
  float last_joints[2];
  float p_cov[6][6];
};

Status populate_odom(struct WheelOdometry* wheel_odom, struct JointState* joint_states, struct Odometry* odom);

#endif 
