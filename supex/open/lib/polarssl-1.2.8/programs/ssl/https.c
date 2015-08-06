/*
 *  SSL client demonstration program
 *
 *  Copyright (C) 2006-2011, Brainspark B.V.
 *
 *  This file is part of PolarSSL (http://www.polarssl.org)
 *  Lead Maintainer: Paul Bakker <polarssl_maintainer at polarssl.org>
 *
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "polarssl/config.h"

#include "polarssl/net.h"
#include "polarssl/ssl.h"
#include "polarssl/entropy.h"
#include "polarssl/ctr_drbg.h"
#include "polarssl/error.h"
#include "polarssl/certs.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


#define DEBUG_LEVEL 1

#define WF_STEP_SIZE 1024
#define RF_STEP_SIZE 1024
#define RF_ARRAY 20
unsigned char GW_BUFF[WF_STEP_SIZE];
unsigned char GR_BUFF[RF_ARRAY][RF_STEP_SIZE];
int GR_SPLIT_SIZE[RF_ARRAY];


void my_debug( void *ctx, int level, const char *str )
{
    if( level < DEBUG_LEVEL )
    {
        fprintf( (FILE *) ctx, "%s", str );
        fflush(  (FILE *) ctx  );
    }
}

#if !defined(POLARSSL_BIGNUM_C) || !defined(POLARSSL_ENTROPY_C) ||  \
    !defined(POLARSSL_SSL_TLS_C) || !defined(POLARSSL_SSL_CLI_C) || \
    !defined(POLARSSL_NET_C) || !defined(POLARSSL_RSA_C) ||         \
    !defined(POLARSSL_CTR_DRBG_C)
static int open_https(lua_State *L)
{
    ((void) argc);
    ((void) argv);

    printf("POLARSSL_BIGNUM_C and/or POLARSSL_ENTROPY_C and/or "
           "POLARSSL_SSL_TLS_C and/or POLARSSL_SSL_CLI_C and/or "
           "POLARSSL_NET_C and/or POLARSSL_RSA_C and/or "
           "POLARSSL_CTR_DRBG_C not defined.\n");
    return( 0 );
}
#else
/*host port request len*/
static int open_https(lua_State *L)
{
    unsigned char *w_buf = NULL;
    unsigned char *r_buf = NULL;
    int ret, size, server_fd;
    const char *pers = "ssl_client1";
    const char *host,*request;
    int port,len,id,short_len;
    int ok = 0;
    int round = 0;
    int sdlen = 0;
    int lvlen = 0;

    entropy_context entropy;
    ctr_drbg_context ctr_drbg;
    ssl_context ssl;
    x509_cert cacert;

    /*
     * get request
     */
    if(lua_gettop(L) != 4)
	    return 0;
    host = lua_tostring(L, 1);
    port = lua_tointeger(L, 2);
    request = lua_tostring(L, 3);
    len = lua_tointeger(L, 4);


    /*
     * 0. Initialize the RNG and the session data
     */
    memset( &ssl, 0, sizeof( ssl_context ) );
    memset( &cacert, 0, sizeof( x509_cert ) );

    printf( "\n  . Seeding the random number generator..." );
    fflush( stdout );

    entropy_init( &entropy );
    if( ( ret = ctr_drbg_init( &ctr_drbg, entropy_func, &entropy,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 )
    {
        printf( " failed\n  ! ctr_drbg_init returned %d\n", ret );
        goto exit;
    }

    printf( " ok\n" );

    /*
     * 0. Initialize certificates
     */
    printf( "  . Loading the CA root certificate ..." );
    fflush( stdout );

#if defined(POLARSSL_CERTS_C)
    ret = x509parse_crt( &cacert, (const unsigned char *) test_ca_crt,
                         strlen( test_ca_crt ) );
#else
    ret = 1;
    printf("POLARSSL_CERTS_C not defined.");
#endif

    if( ret < 0 )
    {
        printf( " failed\n  !  x509parse_crt returned -0x%x\n\n", -ret );
        goto exit;
    }

    printf( " ok (%d skipped)\n", ret );

    /*
     * 1. Start the connection
     */
    printf( "  . Connecting to tcp/%s/%4d...", host, port );
    fflush( stdout );

    if( ( ret = net_connect( &server_fd, host, port ) ) != 0 )
    {
        printf( " failed\n  ! net_connect returned %d\n\n", ret );
        goto exit;
    }

    printf( " ok\n" );

    /*
     * 2. Setup stuff
     */
    printf( "  . Setting up the SSL/TLS structure..." );
    fflush( stdout );

    if( ( ret = ssl_init( &ssl ) ) != 0 )
    {
        printf( " failed\n  ! ssl_init returned %d\n\n", ret );
        goto exit;
    }

    printf( " ok\n" );

    ssl_set_endpoint( &ssl, SSL_IS_CLIENT );
    ssl_set_authmode( &ssl, SSL_VERIFY_OPTIONAL );
    ssl_set_ca_chain( &ssl, &cacert, NULL, "PolarSSL Server 1" );

    ssl_set_rng( &ssl, ctr_drbg_random, &ctr_drbg );
    ssl_set_dbg( &ssl, my_debug, stdout );
    ssl_set_bio( &ssl, net_recv, &server_fd,
                       net_send, &server_fd );

    /*
     * 4. Handshake
     */
    printf( "  . Performing the SSL/TLS handshake..." );
    fflush( stdout );

    while( ( ret = ssl_handshake( &ssl ) ) != 0 )
    {
        if( ret != POLARSSL_ERR_NET_WANT_READ && ret != POLARSSL_ERR_NET_WANT_WRITE )
        {
            printf( " failed\n  ! ssl_handshake returned -0x%x\n\n", -ret );
            goto exit;
        }
    }

    printf( " ok\n" );

    /*
     * 5. Verify the server certificate
     */
    printf( "  . Verifying peer X.509 certificate..." );

    if( ( ret = ssl_get_verify_result( &ssl ) ) != 0 )
    {
        printf( " failed\n" );

        if( ( ret & BADCERT_EXPIRED ) != 0 )
            printf( "  ! server certificate has expired\n" );

        if( ( ret & BADCERT_REVOKED ) != 0 )
            printf( "  ! server certificate has been revoked\n" );

        if( ( ret & BADCERT_CN_MISMATCH ) != 0 )
            printf( "  ! CN mismatch (expected CN=%s)\n", "PolarSSL Server 1" );

        if( ( ret & BADCERT_NOT_TRUSTED ) != 0 )
            printf( "  ! self-signed or not signed by a trusted CA\n" );

        printf( "\n" );
    }
    else
        printf( " ok\n" );

    /*
     * 3. Write the GET request
     */
    printf( "  > Write to server:" );
    fflush( stdout );

    if(len <= WF_STEP_SIZE){
	    size = WF_STEP_SIZE;
	    w_buf = GW_BUFF;
    }else{
	    size = WF_STEP_SIZE * (len/WF_STEP_SIZE + (len%WF_STEP_SIZE > 0 ? 1: 0));
	    w_buf = (unsigned char *)malloc(size);
    }
    memset(w_buf, 0, size);
    memcpy(w_buf, request, len);

    sdlen = 0;
    lvlen = len;
    while( lvlen > 0 ){
	    ret = ssl_write( &ssl, w_buf + sdlen, lvlen );
	    if (ret <= 0 ){
		    if( ret != POLARSSL_ERR_NET_WANT_READ && ret != POLARSSL_ERR_NET_WANT_WRITE )
		    {
			    printf( " failed\n  ! ssl_write returned %d\n\n", ret );
			    goto exit;
		    }
	    }else{
		    sdlen += ret;
		    lvlen -= ret;
		    printf( " %d bytes written\n", ret);
	    }
    }

    /*
     * 7. Read the HTTP response
     */
    printf( "  < Read from server:" );
    fflush( stdout );


    len = 0;
    size = 0;
    round = 0;
    for(id=0;id<RF_ARRAY;id++){
	    memset(GR_BUFF[id], 0, RF_STEP_SIZE);
	    memset(GR_SPLIT_SIZE, 0, sizeof(int)*RF_ARRAY);
    }
    do
    {
		if (round == RF_ARRAY) {
			if (len == 0){
				r_buf = (unsigned char *)malloc(size);
				memset(r_buf, 0, size);
			}
			else{
				r_buf = (unsigned char *)realloc(r_buf, len + size);
			}
			for(id=0;id<RF_ARRAY;id++){
				short_len = GR_SPLIT_SIZE[id];
				if(short_len > 0){
					memcpy(r_buf + len, GR_BUFF[id], short_len);
					len += short_len;
					memset(GR_BUFF[id], 0, RF_STEP_SIZE);
					GR_SPLIT_SIZE[id] = 0;
				}
			}
			size = 0;
			round = 0;
		}
	    ret = ssl_read( &ssl, GR_BUFF[round], RF_STEP_SIZE);

	    if( ret == POLARSSL_ERR_NET_WANT_READ || ret == POLARSSL_ERR_NET_WANT_WRITE )
		    continue;

	    if( ret == POLARSSL_ERR_SSL_PEER_CLOSE_NOTIFY )
		    break;

	    if( ret < 0 )
	    {
		    printf( "failed\n  ! ssl_read returned %d\n\n", ret );
		    break;
	    }

	    if( ret == 0 )
	    {
		    printf( "\n\nEOF\n\n" );
		    break;
	    }
	    GR_SPLIT_SIZE[round] = ret;
	    size = size + ret;
	    round++;
	    printf("----------%d byte recived!\n", ret);
    }
    while( 1 );
    if (size > 0){
	    if (len == 0){
		    r_buf = (unsigned char *)malloc(size);
		    memset(r_buf, 0, size);
	    }
	    else{
		    r_buf = (unsigned char *)realloc(r_buf, len + size);
	    }
	    for(id=0;id<round;id++){
		    short_len = GR_SPLIT_SIZE[id];
		    if(short_len > 0){
			    memcpy(r_buf + len, GR_BUFF[id], short_len);
			    len += short_len;
			    memset(GR_BUFF[id], 0, RF_STEP_SIZE);
		    }
	    }
	    size = 0;
	    round = 0;
    }
    printf( " %d bytes read\n\n%s", len, (char *) r_buf );
    lua_pushlstring(L, (const char *)r_buf, len);
    ok = 1;

    ssl_close_notify( &ssl );

exit:

#ifdef POLARSSL_ERROR_C
    if( ret != 0 )
    {
	    char error_buf[100];
	    error_strerror( ret, error_buf, 100 );
	    printf("Last error was: %d - %s\n\n", ret, error_buf );
    }
#endif

    x509_free( &cacert );
    net_close( server_fd );
    ssl_free( &ssl );

    memset( &ssl, 0, sizeof( ssl ) );

#if defined(_WIN32)
    printf( "  + Press Enter to exit this program.\n" );
    fflush( stdout ); getchar();
#endif
    /*free write buff*/
    if( w_buf != GW_BUFF )
	    free(w_buf);
    /*free read buff*/
    if( r_buf != NULL )
	    free(r_buf);

    if(ok)
	   return 1;
    else
    	return 0;
}


static const struct luaL_Reg lib[] =
{
        {"https",open_https},
        {NULL,NULL}
};

int luaopen_libhttps(lua_State *L) {
	luaL_register(L, "libhttps", lib);
	return 1;
}


#if 0
int main( int argc, char *argv[] )
{
	/*
	   if(argc != 5){
	   	printf("USE like:	./exe [host] [port] [request] [len]\n");
	   	return -1;
	   }
	   char *host = argv[1];
	   int port = atoi(argv[2]);
	   char *request = argv[3];
	   int len = atoi(argv[4]);
	   */

	open_https("sso.cisco.com", 443, "GET /autho/forms/CDClogin.html HTTP/1.0\r\n\r\n", 43);
	return 0;
}
#endif





#endif /* POLARSSL_BIGNUM_C && POLARSSL_ENTROPY_C && POLARSSL_SSL_TLS_C &&
          POLARSSL_SSL_CLI_C && POLARSSL_NET_C && POLARSSL_RSA_C &&
          POLARSSL_CTR_DRBG_C */
