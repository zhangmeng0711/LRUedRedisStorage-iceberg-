local ffi = require "ffi"
local api=ffi.load("/usr/local/openresty/nginx/c/libhash.so")
ffi.cdef[[
        const char* readGetPath(const char key[100], int slot);
        const char* writeGetPath(const char key[100], int slot);
        const char* getHost(const char key[100], const char file[100]);
]]
return api
