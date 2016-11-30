#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "utils.h"
#include "common.h"
#include "wheel_odometry.h"

/* 
 * TODO Better error handling 
 */

/*
 * Initialize WheelOdometry.
 */
Status init(struct WheelOdometry* wheel_odom)
{
  int i=0;
  int j=0;
  for (; i< 6; i++)
  {
    for(; j<6 ; j++)
      wheel_odom->p_cov[i][j] = 0;
  }
  wheel_odom->p_cov[0][0] = .001;
  wheel_odom->p_cov[1][1] = .001;
  wheel_odom->p_cov[2][2] = .01;
  wheel_odom->p_cov[5][5] = .01;
  return SUCCESS;
}

/*
 * Populate Odometry from current accumulated wheel odometry.
 */
Status populate_odom(struct WheelOdometry* wheel_odom, struct JointState* joint_states, struct Odometry* odom)
{    
  float arc_dist = (joint_states->joints[0] - wheel_odom->last_joints[0])/CLICKS_PER_METER;
  float angle_diff = (joint_states->steering_angles[0] - joint_states->steering_angles[1]) * .5;
  float angle_sum = (joint_states->steering_angles[0] + joint_states->steering_angles[1]) * .5;
  float turn_radius = WHEEL_BASE_LENGTH * (tan(3.14/2 - angle_diff));
  float dtheta = arc_dist/turn_radius; 
  wheel_odom->theta += dtheta;
  
  struct Pol pol;
  pol.rho = arc_dist;
  pol.phi = angle_sum;
  struct Cart cart;
  pol2cart(&pol, &cart);
  
  /* world frame */
  wheel_odom->x += cart.x*cos(wheel_odom->theta) - cart.y*sin(wheel_odom->theta);
  wheel_odom->y += cart.x*sin(wheel_odom->theta) + cart.y*cos(wheel_odom->theta);

  /* Populate odom */
  odom->pose_with_cov.pose.point.x = wheel_odom->x;
  odom->pose_with_cov.pose.point.y = wheel_odom->y;

  /* Compute quaternion */
  quaternion_from_euler(0, 0, wheel_odom->theta, &(odom->pose_with_cov.pose.quaternion));
    
  /* Update state */
  wheel_odom->last_joints[0] = joint_states->joints[0];
  wheel_odom->last_joints[1] = joint_states->joints[1];

  return SUCCESS;
}

