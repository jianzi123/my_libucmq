#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define CONV_STEP_COUNT 1024
unsigned char G_UTF8_BUFF[CONV_STEP_COUNT*3 + 1];
unsigned short G_UNC16_BUFF[CONV_STEP_COUNT + 1];
unsigned char GBK3121_BUF[CONV_STEP_COUNT*3 + 1];

static unsigned char *unicode_to_utf8(unsigned short *unicode_buf, int conv_count, int endian)
{
	unsigned short *uni_ptr = unicode_buf;
	unsigned char *utf_ptr = NULL;
	unsigned short word;
	unsigned char ch;
	unsigned int uni_ind = 0, utf_ind = 0, utf_num = 0;
	if (conv_count > CONV_STEP_COUNT){
		utf_ptr = (unsigned char *)malloc(conv_count * 3 + 1);
		memset(utf_ptr, 0, conv_count * 3 + 1);
	}
	else{
		utf_ptr = G_UTF8_BUFF;
		memset(utf_ptr, 0, CONV_STEP_COUNT*3 + 1);
	}
//*unicode_buf = 5*16*16*16 + 9*16*16 + 2*16 + 7;

	while(1)
	{
		word = *(uni_ptr + uni_ind);
		uni_ind++;

		if(word == 0x0000)  //结束符
			break;

		if(endian == 1)  //大端
		{
			//高低位交换
			ch = (unsigned char)word;
			word = word >> 8;
			word += ch << 8;
		}

		if(word < 0x80)
		{
			*(utf_ptr + utf_ind) = (word & 0x7F) | 0x00;
			utf_ind++;

			utf_num++;
		}
		else if(word < 0x0800)
		{
			*(utf_ptr + utf_ind) = ((word >> 6) & 0x1F) | 0xC0;
			utf_ind++;
			*(utf_ptr + utf_ind) = (word & 0x3F) | 0x80;
			utf_ind++;

			utf_num++;
		}
		else if(word < 0x010000)
		{
			*(utf_ptr + utf_ind) = ((word >> 12) & 0x0F) | 0xE0;
			utf_ind++;
			*(utf_ptr + utf_ind) = ((word >> 6) & 0x3F) | 0x80;
			utf_ind++;
			*(utf_ptr + utf_ind) = (word & 0x3F) | 0x80;
			utf_ind++;

			utf_num++;
		}
		else if(word < 0x110000)
		{
			*(utf_ptr + utf_ind) = ((word >> 18) & 0x07) | 0xF0;
			utf_ind++;
			*(utf_ptr + utf_ind) = ((word >> 12) & 0x3F) | 0x80;
			utf_ind++;
			*(utf_ptr + utf_ind) = ((word >> 6) & 0x3F) | 0x80;
			utf_ind++;
			*(utf_ptr + utf_ind) = (word & 0x3F) | 0x80;
			utf_ind++;

			utf_num++;
		}

		if(utf_num > conv_count - 1)
		{
			break;
		}
	}

	return utf_ptr;
}

static int char_to_number(char str)
{
	switch(str){
		case 'a':
			return 10;
		case 'b':
			return 11;
		case 'c':
			return 12;
		case 'd':
			return 13;
		case 'e':
			return 14;
		case 'f':
			return 15;
		default:
			return str - '0';
	}

}

static int unc16_to_utf8(lua_State *L)
{
	const char *unicode_buf = lua_tostring(L, 1);
	int conv_count = lua_tointeger(L, 2);
	int endian = lua_tointeger(L, 3);

	unsigned short *unc16_ptr = NULL;
	unsigned char *result = NULL;
	int nb1,nb2,nb3,nb4;
	int i = 0;
	int step = 0;

	if (conv_count > CONV_STEP_COUNT){
		unc16_ptr = (unsigned short *)malloc(conv_count * sizeof(unsigned short) + 1);
		memset(unc16_ptr, 0, conv_count * sizeof(unsigned short) + 1);
	}
	else{
		unc16_ptr = G_UNC16_BUFF;
		memset(unc16_ptr, 0, CONV_STEP_COUNT + 1);
	}

	for(i = 0, step = 0;i < conv_count; i++){
		/*
		if ( (*(unicode_buf + i*6 + 0) == '\\') && (*(unicode_buf + i*6 + 1) == 'u') ){
			nb1 = char_to_number(*(unicode_buf + i*6 + 2));
			nb2 = char_to_number(*(unicode_buf + i*6 + 3));
			nb3 = char_to_number(*(unicode_buf + i*6 + 4));
			nb4 = char_to_number(*(unicode_buf + i*6 + 5));
			*(unc16_ptr + i) = nb1*16*16*16 + nb2*16*16 + nb3*16 + nb4;
		}*/
		if ( *(unicode_buf + step) == '\0' )
			break;

		if ( (*(unicode_buf + step) == '\\') && (*(unicode_buf + step + 1) == 'u') ){
			nb1 = char_to_number(*(unicode_buf + step + 2));
			nb2 = char_to_number(*(unicode_buf + step + 3));
			nb3 = char_to_number(*(unicode_buf + step + 4));
			nb4 = char_to_number(*(unicode_buf + step + 5));
			*(unc16_ptr + i) = nb1*16*16*16 + nb2*16*16 + nb3*16 + nb4;
			step = step + 6;
		}
		else{
			*(unc16_ptr + i) = *(unicode_buf + step);
			step = step + 1;
		}
	}

	result = unicode_to_utf8(unc16_ptr, conv_count, endian);

	lua_pushlstring(L, (const char *)result, strlen(result));
	if (conv_count > CONV_STEP_COUNT){
		free(unc16_ptr);
		free(result);
	}
	return 1;
}

static int luastr_to_ctype(lua_State *L)
{
	const char *luastring = lua_tostring(L, 1);
	const char *type = lua_tostring(L, 2);
	//int count = lua_tointeger(L, 3);

	if ( !strcmp( type, "int" ) ){
		int val = *((int *)luastring);
		lua_pushinteger(L, val);
		return 1;
	}
	return 0;
}

static int ConvertCode(char *sFromCharset, char *sToCharset, char *sInBuf, int nInLen, char *sOutBuf, int nOutLen)
{
        iconv_t cd;
        char **pIn = (&sInBuf);
        char **pOut = &sOutBuf;

        cd = iconv_open(sToCharset, sFromCharset);
        if (cd == 0)
                return -1;

        memset(sOutBuf, 0, nOutLen);
        if ((int)iconv(cd, pIn, (size_t *)&nInLen, pOut, (size_t *)&nOutLen) == -1)
                return -2;

        iconv_close(cd);

        return 0;
}

static int u2g(lua_State *L) {
        char *sInBuf = (char *)lua_tostring(L, 1);
        int nInLen = lua_tointeger(L, 2);
        printf("\n%s\n", sInBuf);
	int nOutLen = 9 * nInLen;
        //char *sOutBuf = (char *)calloc(nOutLen, sizeof(unsigned char));
	char *sOutBuf = GBK3121_BUF;
        ConvertCode("utf-8", "gb2312", sInBuf, nInLen, sOutBuf, nOutLen);
        lua_pushlstring(L, (const char *)sOutBuf, strlen(sOutBuf));
/*
	if (sOutBuf)
		free(sOutBuf);
*/
        return 1;
}

static int g2u(lua_State *L) {
        char *sInBuf = (char *)lua_tostring(L, 1);
        int nInLen = lua_tointeger(L, 2);
	int nOutLen = 6 * nInLen;
        //char *sOutBuf = (char *)calloc(nOutLen, sizeof(unsigned char));
	char *sOutBuf = GBK3121_BUF;
        ConvertCode("gb2312", "utf-8", sInBuf, nInLen, sOutBuf, nOutLen);
        lua_pushlstring(L, (const char *)sOutBuf, strlen(sOutBuf));
        /*
	if (sOutBuf)
		free(sOutBuf);
                */
        return 1;
}

static const struct luaL_Reg lib[] =
{
	{"unc16_to_utf8", unc16_to_utf8},
	{"luastr_to_ctype", luastr_to_ctype},
	{"u2g", u2g},
	{"g2u", g2u},
	{NULL, NULL}
};

int luaopen_conv(lua_State *L) {
	luaL_register(L, "conv", lib);
	return 1;
}
