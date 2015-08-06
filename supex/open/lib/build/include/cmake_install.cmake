# Install script for directory: /home/cheng/company/supex/open/lib/polarssl-1.2.8/include

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "0")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/polarssl" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/ssl_cache.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/bignum.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/pkcs12.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/entropy.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/md_wrap.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/aes.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/asn1write.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/asn1.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/timing.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/dhm.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/pkcs11.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/xtea.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/x509write.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/net.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/cipher_wrap.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/entropy_poll.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/ctr_drbg.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/pem.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/openssl.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/gcm.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/padlock.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/sha2.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/md5.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/md.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/bn_mul.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/cipher.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/pkcs5.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/config.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/debug.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/version.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/havege.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/x509.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/md4.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/camellia.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/error.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/certs.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/blowfish.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/arc4.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/ssl.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/des.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/pbkdf2.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/md2.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/sha1.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/rsa.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/base64.h"
    "/home/cheng/company/supex/open/lib/polarssl-1.2.8/include/polarssl/sha4.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

