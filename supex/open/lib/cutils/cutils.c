/*
 * auth: coanor
 * date: Sat Aug  3 10:20:26 CST 2013
 */
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <uuid/uuid.h>

#define g_pi  3.1415926

// utils
static char char_to_num(char ch) {
    unsigned char c;
    if (ch >= '0' && ch <= '9')
        c = ch - '0';
    else
        c = toupper(ch) - 'A' + 10;
    return c;
}

static char *url_encode(const char *src) {
    char *out = calloc(1, strlen(src) * 3 + 1);
    char *p = out;
    const char *q = src;
    char buf[4] = {0};

    while (*q != '\0') {
        switch (*q) {
            case '@' : case '#' : case '$' : case '%' :
            case '^' : case '&' : case '+' : case '|' :
            case '}' : case '{' : case '"' : case ':' :
            case '?' : case '>' : case '<' : case '[' :
            case ']' : case '\\': case '\'' : case ';':
            case '/' : case ',' : case ' ' : case '`' :
            case '\r': case '\n':
                sprintf(buf, "%%%02X", *q);
                strcat(p, buf);
                p += 3;
                ++q;
                break;
            default:
                if ((unsigned char )*q > 0x7f) {
                    /* unicode */
                    sprintf(buf, "%%%X", (unsigned char)*q);
                    strcat(p, buf);
                    p += 3;
                    ++q;
                } else {
                    *p = *q;
                    ++p;
                    ++q;
                }
                break;
        }
    }
    return out;
}

static char *url_decode(const char *src) {
    char *out = calloc(1, strlen(src) + 1);
    const char *p = src;
    char *q = out;

    char byte1, byte2;
    while (*p != '\0') {
        switch (*p) {
            case '%':
                byte1 = char_to_num(p[1]);
                byte2 = char_to_num(p[2]);
                *q = ((byte1 << 4) | byte2);
                p += 3;
                ++q;
                break;
            default:
                *q = *p;
                ++q;
                ++p;
        }
    }
    return out;
}

// api
static int lua_url_encode_C(lua_State *L) {
    const char *src = luaL_checkstring(L, 1);
    char *out = url_encode(src);
    lua_pushstring(L, out);
    free(out);
    return 1;

}

// api
static int lua_url_decode_C(lua_State *L) {
    const char *src = luaL_checkstring(L, -1);
    char *out = url_decode(src);
    lua_pushstring(L, out);
    free(out);
    return 1;

}

// api
static int lua_msleep_C(lua_State *L) {
    long ms = luaL_checknumber(L, 1);
    usleep(1000 * ms);
    return 0;
}


/* 得到中央经线经度
*/
static double GetCentralMeridian(double dL) {
    double dBeltNumber = floor((dL + 1.5) / 3.0);
    double dL0 = dBeltNumber * 3.0;

    return dL0;
}


/* 计算地球上两点之间的距离
   / <param name="dLongitude1">第一点的经度</param>
   / <param name="dX1">第一点的X值</param>
   / <param name="dY1">第一点的Y值</param>
   / <param name="dLongitude2">第二点的经度</param>
   / <param name="dX2">第二点的X值</param>
   / <param name="dY2">第二点的Y值</param>
   */
static double CalculateTwoPointsDistance(double dLongitude1, double dX1, double dY1,
        double dLongitude2, double dX2, double dY2) {
    double dL01 = GetCentralMeridian(dLongitude1);
    double dL02 = GetCentralMeridian(dLongitude2);
    double dCentralDelta = fabs(dL02 - dL01);
    double dDeltaX = 0.0;
    if (dLongitude2 > dLongitude1)
        dDeltaX = dX2 - dX1 + dCentralDelta * 100* 954;
    else
        dDeltaX = dX1 - dX2 + dCentralDelta * 100 * 954;
    double dDistance = sqrt(pow(dDeltaX, 2) + pow(dY2 - dY1, 2));
    return dDistance;
}

/* 根据经纬度转换为投影坐标
*/
static void BL2XY(double dB, double dL, double *dX, double *dY) {
    double dBeltNumber = floor((dL + 1.5) / 3.0);
    double dL0 = dBeltNumber * 3.0;

    double a = 6378137;
    double b = 6356752.3142;
    double k0 = 1;
    double FE = 500000;

    double e1 = sqrt(1 - pow(b / a, 2));
    double e2 = sqrt(pow(a/b, 2) - 1);

    dB = (dB * g_pi) / 180.0;
    double T = pow(tan(dB), 2);

    double C = e2 * e2 * pow(cos(dB), 2);

    dL = dL * g_pi / 180.0;
    dL0 = dL0 * g_pi / 180.0;
    double A = (dL - dL0) * cos(dB);

    double M = (1 - pow(e1, 2) / 4.0 - 3.0 * pow(e1, 4) / 64.0 - 5.0 * pow(e1, 6) / 256.0) * dB;
    M = M - (3.0 * pow(e1, 2) / 8.0 + 3.0 * pow(e1, 4) / 32.0 + 45.0 * pow(e1, 6) / 1024.0) * sin(dB * 2);
    M = M + (15.0 * pow(e1, 4) / 256.0 + 45.0 * pow(e1, 6) / 1024.0) * sin(dB * 4);
    M = M - (35.0 * pow(e1, 6) / 3072.0) * sin(dB * 6);
    M = a * M;

    double N = a / pow(1.0 - pow(e1, 2) * pow(sin(dB), 2), 0.5);

    double mgs1 = pow(A, 2) / 2.0;
    double mgs2 = pow(A, 4) / 24.0 * (5.0 - T + 9.0 * C + 4.0 * pow(C, 2));
    double mgs3 = pow(A, 6) / 720.0 * (61.0 - 58.0 * T + pow(T, 2) + 270 * C - 330.0 * T * C);
    *dY = M + N * tan(dB) * (mgs1 + mgs2) + mgs3;
    *dY = *dY * k0;

    mgs1 = A + (1.0 - T + C) * pow(A, 3) / 6.0;
    mgs2 = (5.0 - 18.0 * T + pow(T, 2) + 14.0 * C - 58.0 * T * C) * pow(A, 5) / 120.0;
    *dX = (mgs1 + mgs2) * N * k0 + FE;
}

// api
static int bl2xy(lua_State *ls) {
    double lon = luaL_checknumber(ls, 1);
    double lat = luaL_checknumber(ls, 2);

    double x, y;

    BL2XY(lat, lon, &x, &y);

    lua_pushnumber(ls, x);
    lua_pushnumber(ls, y);

    return 2;
}

// api
// thre return value's unit is mile
static int dist_C(lua_State *ls) {
    double lon1 = luaL_checknumber(ls, 1);
    double lat1 = luaL_checknumber(ls, 2);
    double lon2 = luaL_checknumber(ls, 3);
    double lat2 = luaL_checknumber(ls, 4);

    //2014-06-09 更新球面2点距离算法
    double dist = 6378137*acos(sin(lat1/57.2958)*sin(lat2/57.2958)+cos(lat1/57.2958)*cos(lat2/57.2958)*cos((lon1-lon2)/57.2958));

    /*
    double dx1, dy1;
    double dx2, dy2;

    BL2XY(lat1, lon1, &dx1, &dy1);
    BL2XY(lat2, lon2, &dx2, &dy2);
    double dist = CalculateTwoPointsDistance(lon1, dx1, dy1, lon2, dx2, dy2);
    */

    lua_pushnumber(ls, dist);
    return 1;
}


static int create_uuid_C(lua_State *L) {

    char str[37] = {0};
    uuid_t uuid;

    uuid_generate_time(uuid);
    uuid_unparse(uuid, str);

    lua_pushstring(L, str);

    return 1;
}

static const luaL_Reg lib[] = {
    {"url_encode", lua_url_encode_C},
    {"url_decode", lua_url_decode_C},
    {"msleep", lua_msleep_C},
    {"gps_distance", dist_C},
    {"bl2xy", bl2xy},
    {"uuid", create_uuid_C},
    {NULL, NULL}
};


int luaopen_cutils(lua_State *L) {

    luaL_register(L, "cutils", lib);

    return 0;
}
