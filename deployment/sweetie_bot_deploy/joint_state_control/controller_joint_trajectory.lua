--
-- VIRTUAL ROBOT and JOINT STATE controllers
--
-- controllers: AnimJointTrajectory
--
-- Intended to be run via config script.
--
controller = controller or {}

ros:import("sweetie_bot_controllers_joint_space")

-- 
-- load AnimJointTrajectory controller
--
depl:loadComponent("controller_joint_trajectory", "sweetie_bot::motion::controller::AnimJointTrajectory")
controller.joint_trajectory = depl:getPeer("controller_joint_trajectory")
rttlib_extra.get_peer_rosparams(controller.joint_trajectory)
-- register controller
resource_control.register_controller(controller.joint_trajectory)
-- timer
controller.joint_trajectory:getProperty("period"):set(timer.period);
depl:connect(timer.controller.port, "controller_joint_trajectory.sync", rtt.Variable("ConnPolicy"))
-- data flow: controller <-> agregator_ref
depl:connect("controller_joint_trajectory.out_joints_ref_fixed", "agregator_ref.in_joints", rtt.Variable("ConnPolicy"))
depl:connect("agregator_ref.out_joints_sorted", "controller_joint_trajectory.in_joints_sorted", rtt.Variable("ConnPolicy"))
-- configure actionlib
ros:import("rtt_actionlib")
controller.joint_trajectory:loadService("actionlib")
controller.joint_trajectory:provides("actionlib"):connect("~controller_joint_trajectory")
-- connect to RobotModel
depl:connectServices("controller_joint_trajectory", "agregator_ref")
-- prepare to start
assert(controller.joint_trajectory:configure())
