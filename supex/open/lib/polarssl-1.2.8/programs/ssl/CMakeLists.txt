set(libs
     polarssl
)

if(USE_PKCS11_HELPER_LIBRARY)
    set(libs ${libs} pkcs11-helper)
endif(USE_PKCS11_HELPER_LIBRARY)

if(ENABLE_ZLIB_SUPPORT)
    set(libs ${libs} ${ZLIB_LIBRARIES})
endif(ENABLE_ZLIB_SUPPORT)

add_executable(ssl_client1 ssl_client1.c)
target_link_libraries(ssl_client1 ${libs})

#https
include_directories(/usr/local/include/luajit-2.0/)
add_library(https SHARED https.c )
set_target_properties(https PROPERTIES OUTPUT_NAME "https")
target_link_libraries(https ${libs})

add_executable(ssl_client2 ssl_client2.c)
target_link_libraries(ssl_client2 ${libs})

add_executable(ssl_server ssl_server.c)
target_link_libraries(ssl_server ${libs})

add_executable(ssl_server2 ssl_server2.c)
target_link_libraries(ssl_server2 ${libs})

add_executable(ssl_fork_server ssl_fork_server.c)
target_link_libraries(ssl_fork_server ${libs})

add_executable(ssl_mail_client ssl_mail_client.c)
target_link_libraries(ssl_mail_client ${libs})

install(TARGETS ssl_client1 ssl_client2 ssl_server ssl_fork_server ssl_mail_client
        DESTINATION "bin"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
