-- Auther:GengXuanxuan
-- Date:2014-04-13
-- Feature:#2267
--
-- GPSPoint class

module("gps_point", package.seeall)

GPSPoint = {

}

-- Return new object of GPSPoint
GPSPoint.new = function(self)
        local attr = {
        }

        setmetatable(attr, self)
        self.__index = self
        self.__tostring = self.tostring

        return attr
end

-- Init GPSPoint from request body
GPSPoint.init = function(self, arg)
        self['longitude']    = arg['longitude'] or -1 
        self['latitude']     = arg['latitude'] or -1
        self['speed']        = arg['speed'] or 0
        self['GPSTime']      = arg['GPSTime'] or -1
        self['direction']    = arg['direction'] or -1
end

-- Return string of GPSPoint
GPSPoint.tostring = function(self)
        local str = "{ longitude={" .. self['longitude'] .. "}, latitude={" .. self['latitude'] ..
                        "}, speed={" .. self['speed'] .. "}, GPSTime={" .. self['GPSTime'] ..
                        "}, direction={" .. self['direction'] .. "} }"

        return str
end
