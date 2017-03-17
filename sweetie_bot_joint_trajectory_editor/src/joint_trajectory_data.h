#ifndef JOINT_TRAJECTORY_POINT_LIST_H
#define JOINT_TRAJECTORY_POINT_LIST_H

#include <ros/ros.h>

#include <sensor_msgs/JointState.h>
#include <control_msgs/FollowJointTrajectoryGoal.h>

namespace sweetie_bot {
namespace interface {

/*
FollowJointTrajectoryGoal:

trajectory:
  header: {secu: 0, stamp: 0.0, frame_id: ""}
  joint_names:
  - 'joint11'
  points:
  - positions: [0.0]
    velocities: [0.0]
    accelerations: [0.0]
    effort: [0.0]
    time_from_start: {secs: 0, nsecs: 0}
path_tolerance:
- {name: 'joint11', position: 0.1, velocity: 0.1, acceleration: 0.1}
goal_tolerance:
- {name: 'joint11', position: 0.1, velocity: 0.1, acceleration: 0.1}
goal_time_tolerance: {secs: 0, nsecs: 0}
*/

class JointTrajectoryData
{

	public:
		typedef sensor_msgs::JointState JointState;
		typedef control_msgs::FollowJointTrajectoryGoal FollowJointTrajectoryGoal;
		typedef control_msgs::JointTolerance JointTolerance;
		
		std::vector<std::string> joint_names_;
		FollowJointTrajectoryGoal follow_joint_trajectory_goal_;
		bool loadFromMsg(const FollowJointTrajectoryGoal& follow_joint_trajectory_goal);
		bool addJoint(const std::string name);
		bool removeJoint(const std::string name);
		bool removeJoint(const int row);
	protected:

		struct Tolerances {
			JointTolerance path_tolerance;
			JointTolerance goal_tolerance;
		};
		std::map<std::string, Tolerances> tolerances_;
		double goal_time_;
		std::vector<double> path_tolerance_; 
		std::vector<double> goal_tolerance_;
		double goal_time_tolerance_;
	public:
		JointTrajectoryData(const FollowJointTrajectoryGoal& follow_joint_trajectory_goal);
    bool addPoint(const JointState& msg, double time_from_start);
};

} // namespace interface
} // namespace sweetie_bot

#endif // JOINT_TRAJECTORY_POINT_LIST_H
