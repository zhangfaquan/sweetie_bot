require 'rttlib'

rttlib_extra = {}

--- Per property setter for complex types
function rttlib_extra.set_property(peer, prop, prop_type, value) 
	var = rtt.Variable(prop_type);
	var:fromtab(value)
	peer:getProperty(prop):set(var)
end

--- Get string property as string from ROS Parameter Server (exec rosparam)
function rttlib_extra.rosparam_get_string(param)
	local f = assert(io.popen("rosparam get " .. param .. " 2>&1", 'r'))
	local s = assert(f:read('*a'))
	if string.find(s, "^ERROR") then 
		-- TODO normal error handling
		print("ERROR: unable to get param ", param)
		return nil
	else
		s = string.gsub(s, "\n$", "")
		return s
	end
end

--- Get string property as string from ROS Parameter Server (exec rosparam)
function rttlib_extra.get_rosparam(peer)
	peer:loadService("rosparam")
	peer:provides("rosparam"):getAll();
end

