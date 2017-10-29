-- RESOURCE CONTROL DEPLOYMENT MODULE
--
require 'rttlib'
require 'rttlib_extra'

-- get Deployer
local depl = rtt.getTC():getPeer("Deployer")

-- import ros facilities
depl:import("rtt_ros") 
local ros = rtt.provides("ros")

-- import 
ros:import("sweetie_bot_resource_control")

-- Module table
resource_control = {}

-- Load arbiter
depl:loadComponent("resource_control/arbiter", "sweetie_bot::motion::ResourceArbiter")
resource_control.arbiter = depl:getPeer("resource_control/arbiter")
rttlib_extra.get_peer_rosparams(resource_control.arbiter)

-- Connect a requester to the arbiter
function resource_control.register_controller(peer)
	local cp = rtt.Variable("ConnPolicy")
	if type(peer) ~= "string" then
		peer = peer:getName()
	end
	success = depl:loadService(peer, "resource_client")
	success = success and depl:connect("resource_control/arbiter.out_resource_assigment", peer .. ".resource_client.in_resource_assigment", cp )
	success = success and depl:connect("resource_control/arbiter.in_resource_request", peer .. ".resource_client.out_resource_request", cp )
	success = success and depl:connect("resource_control/arbiter.in_resource_requester_status", peer .. ".resource_client.out_resource_requester_status", cp )
	return success
end
