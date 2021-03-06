--
-- VIRTUAL ROBOT and JOINT STATE controllers
--
-- controllers: ExecuteJointTrajectory
--
-- Intended to be run via config script.
--
require "motion_core"

controller = controller or {}

-- 
-- load ExecuteJointTrajectory controller
--
ros:import("sweetie_bot_controller_joint_space")

depl:loadComponent("controller/joint_trajectory", "sweetie_bot::motion::controller::ExecuteJointTrajectory")
controller.joint_trajectory = depl:getPeer("controller/joint_trajectory")
-- load parameters and services
config.get_peer_rosparams(controller.joint_trajectory)
-- register controller
resource_control.register_controller(controller.joint_trajectory)
-- timer
depl:connect(timer.controller.port, "controller/joint_trajectory.sync", rtt.Variable("ConnPolicy"))
-- data flow: controller <-> aggregator_ref
depl:connect("controller/joint_trajectory.out_joints_ref_fixed", "aggregator_ref.in_joints", rtt.Variable("ConnPolicy"))
depl:connect("controller/joint_trajectory.out_supports", "aggregator_ref.in_supports", rtt.Variable("ConnPolicy"))
depl:connect("aggregator_ref.out_joints_sorted", "controller/joint_trajectory.in_joints_sorted", rtt.Variable("ConnPolicy"))
-- configure actionlib
controller.joint_trajectory:loadService("actionlib")
controller.joint_trajectory:provides("actionlib"):connect("~controller/joint_trajectory")
-- connect to RobotModel
depl:connectServices("controller/joint_trajectory", "aggregator_ref")
-- prepare to start
assert(controller.joint_trajectory:configure(), "ERROR: controller joint trajectory is not configured.")
