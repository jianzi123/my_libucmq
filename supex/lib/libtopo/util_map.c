#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "util_map.h"

#define EARTH_RADIUS 6378137
int util_get_length( double from_lon, double from_lat, double to_lon, double to_lat){
	if (fabs(from_lon - to_lon) <= 0.00001 && fabs(from_lat - to_lat) <= 0.00001){
		return 0;
	}
	double distance=0.0;
	distance = EARTH_RADIUS * acos( sin(from_lat / 57.2958 ) * sin(to_lat / 57.2958 ) + cos(from_lat / 57.2958 ) * cos(to_lat / 57.2958) * cos( (from_lon - to_lon) / 57.2958)) ;
	return floor(distance);
}

#define PI_VALUE 3.1415926
double util_get_angle( double from_lon, double from_lat, double to_lon, double to_lat){
	if ( fabs( to_lon - from_lon) <= 0.0000001){
		if (to_lat > from_lat)
			return 0.0;
		else
			return 180.0;
	}
	if ( fabs(to_lat - from_lat) <= 0.0000001){
		if (to_lon > from_lon)
			return 90.0;
		else
			return 270.0; 
	}
	double vtan = atan( (to_lon - from_lon) /(to_lat - from_lat)) /PI_VALUE * 180;
	if (vtan > 0){
		if (to_lat > from_lat)
			return vtan;
		else
			return vtan + 180.0;
	}
	else{
		if (to_lon < from_lon)
			return vtan + 360.0;
		else
			return vtan + 180.0;
	}
}
