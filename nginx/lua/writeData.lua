local key = ngx.var.arg_key
local ffi = require "ffi"
local api = require "lua.hash"
local new_key=ngx.unescape_uri(key)
--[[local host = api.getHost(new_key,"/usr/local/openresty/nginx/lua/ips")
if host ~= "error"
then
	local host_str = ffi.string(host)
	if host_str == "10.50.8.73"
	then]]
		local path = api.writeGetPath(new_key, 1000)
		if path == "error" then
			ngx.say("get path error")
			return 
		end
		local data = ffi.string(path)
		local file_name = data..key
		local file = io.open(file_name, "w")
		io.output(file)
		if file == nil then
			ngx.say("file open fail")
			ngx.say(file_name)
		else
		ngx.req.read_body()
		file:write(ngx.req.get_body_data())
		io.close(file)
		end
		--ngx.say(file_name)
	--[[else
		ngx.req.read_body()
		local res = ngx.location.capture("/write"..host_str,
			{method = ngx.HTTP_POST, args= { key = key},
			body = ngx.req.get_body_data()})
		ngx.say(res.body)
	end
else
	ngx.say("error")
end]]
