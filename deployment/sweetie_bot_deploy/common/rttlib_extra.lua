require 'rttlib'

-- get Deployer
local depl = rtt.getTC():getPeer("Deployer")
-- import ros facilities
depl:import("rtt_ros") 
local ros = rtt.provides("ros")
-- import
ros:import("rtt_rosparam")
depl:loadService("Deployer", "rosparam")

rttlib_extra = {}
rttlib_extra.rosparam = depl:provides("rosparam")

--- Per property setter for complex types
function rttlib_extra.set_property(peer, prop, prop_type, value) 
	var = rtt.Variable(prop_type);
	var:fromtab(value)
	peer:getProperty(prop):set(var)
end

--- Get parameter from ROS Parameter Server
function rttlib_extra.get_rosparam(name, typename)
	typename = string.lower(typename)
	local getOp = rttlib_extra.rosparam:getOperation("get" .. typename:sub(1,1):upper() .. typename:sub(2))
	if getOp then
		local var = rtt.Variable(typename)
		if getOp(name, var) then
			return var:totab()
		end
	end
	return nil
end

function rttlib_extra.get_property(peer, property_name)
	prop_names = peer:getPropertyNames()
	for i, name in pairs(prop_names) do
		if name == property_name then
			return peer:getProperty(name)
		end
	end
	return nil
end


--- Load properties to peer using rosparam service getAll() call.
function rttlib_extra.get_peer_rosparams(peer)
	-- process some properties specifically
	-- `period` property is allways set to `~timer/period'
	prop = rttlib_extra.get_property(peer, "period")
	if prop then
		period = rttlib_extra.get_rosparam("~timer/period", "float")
		if period then
			prop:set(period) 
		else
			print("WARNING: `~timer/period` is paramerer is not set")
		end
	end
	-- use rosparam `priority` to set component RT priority
	priority = rttlib_extra.get_rosparam("~" .. peer:getName() .. "/priority", "int")
	if priority then
		depl:setActivity(peer:getName(), 0, priority, rtt.globals.ORO_SCHED_RT)
	end
	-- load parameters from parameter server
	peer:loadService("rosparam")
	peer:provides("rosparam"):getAll();
end

