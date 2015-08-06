#!/bin/bash

##001. 配置
yum install gd-devel
yum install libpng-devel
yum install libxml2-devel
yum install freetype-devel
yum install libjpeg-turbo-devel
yum install curl-devel
yum install libmcrypt-devel
#yum install libmcrypt

##002. 安装php
tar -zxvf php-5.5.9.tar.gz
cd php-5.5.9
#对php的
./configure --prefix=/data/php \
 --with-png-dir=/usr/lib64 \
 --with-jpeg-dir=/usr/lib64 \
 --with-libxml-dir=/usr/lib64 \
 --with-freetype-dir=/usr/lib64 \
 --with-xpm-dir=/usr/lib64 \
 --enable-mbstring \
 --enable-sockets \
 --enable-soap \
 --enable-zip \
 --enable-fpm \
 --with-gd \
 --with-zlib \
 --with-mcrypt \
 --with-mysql=mysqlnd \
 --with-mysqli=mysqlnd \
 --with-pdo-mysql=mysqlnd
#make && make install


##003. 安装nginx
nginx
./configure  --prefix=/data/nginx
make -j4 & make install

##004. 修改nginx 配置文件
cd /data/nginx/
###把下面的server添加到/data/nginx/conf/nginx.conf
    server {
                listen       8080;
                server_name  driview;
                index index.html index.htm index.php;
                root html;

                location ~ .*\.(php|php5)?$  {
                        fastcgi_pass 127.0.0.1:9000;
                        fastcgi_index index.php;
                        #include fcgi.conf;
                        include fastcgi.conf;
                }

                location ~ .*\.(gif|jpg|jpeg|png|bmp|swf)$ {
                        expires      30d;
                }

                location ~ .*\.(js|css)?$ {
                        expires      24h;
                }

                location /driview {
                        root html;
                }

        }

##004. 修改参数运行
cp autocode /data/ngxin/html/ -r
vim autocode/js/config.js
#把下面语句中的host，修改为driview所在的host(x.x.x.x)
var configArr={"appcenter":["192.168.1.14",9999],"driview":["192.168.1.14",8888]};
修改后
var configArr={"appcenter":["x.x.x.x",9999],"driview":["x.x.x.x",8888]};
cd /data/php/
./sbin/php-fpm 
cd /data/nginx/
./sbin/nginx
#打开下面的网址在浏览器
http://x.x.x.x:8080/autocode/?service=driview
