#ifndef __COMMON__
#define __COMMON__

typedef enum {SUCCESS, FAILURE} Status;

struct Vector
{
    float x;
    float y;
    float z;
};

struct Quaternion
{
    float x;
    float y;
    float z;
    float w;
};

struct Header
{
    const char* frame_id;     
};

struct Pose
{
    struct Vector point;
    struct Quaternion quaternion;
};

struct PoseWithCovariance
{
    struct Pose pose;
    float covariance[36];  
};

struct Twist
{
    struct Vector linear;
    struct Vector angular;
};

struct TwistWithCovariance
{
    struct Twist twist;
    float covariance[36];  
};

struct Odometry
{
    struct Header header;  
    const char* child_frame_id;    
    struct PoseWithCovariance pose_with_cov;
    struct TwistWithCovariance twist_with_cov;
};

struct JointState
{
    float joints[2];
    float steering_angles[2];
};

struct Angle
{
    float roll;
    float pitch;
    float yaw;
};

#endif
