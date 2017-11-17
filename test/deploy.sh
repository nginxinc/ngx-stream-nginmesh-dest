#!/bin/bash
cp test/config/nginx.conf /etc/nginx
rm -rf /etc/nginx/conf.d/*
cp test/config/conf.d/* /etc/nginx/conf.d
node tests/services/http.js 9100 > u1.log 2> u1.err &
node tests/services/tcp-invoke.js 9000 dest > t1.log 2> t1.err
tests/prepare_proxy.sh -p 15001 -u ${DOCKER_USER} &
nginx -s reload