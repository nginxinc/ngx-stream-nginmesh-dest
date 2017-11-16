NGINX_VER = 1.13.5
DOCKER_REPO=nginmesh
RUST_COMPILER_TAG = 1.21.0
UNAME_S := $(shell uname -s)
GIT_COMMIT=$(shell git rev-parse --short HEAD)
export MODULE_DIR=${PWD}
DOCKER_USER=101
NGX_MODULES = --with-compat  --with-threads --with-http_addition_module \
     --with-http_auth_request_module   --with-http_gunzip_module --with-http_gzip_static_module  \
     --with-http_random_index_module --with-http_realip_module --with-http_secure_link_module \
     --with-http_slice_module  --with-http_stub_status_module --with-http_sub_module \
     --with-stream --with-stream_realip_module --with-stream_ssl_preread_module
ifeq ($(UNAME_S),Linux)
    NGINX_SRC += nginx-linux
    NGX_OPT= $(NGX_MODULES) \
       --with-file-aio --with-http_ssl_module --with-stream_ssl_module  \
       --with-cc-opt='-g -fstack-protector-strong -Wformat -Werror=format-security -Wp,-D_FORTIFY_SOURCE=2 -fPIC' \
       --with-ld-opt='-Wl,-Bsymbolic-functions -Wl,-z,relro -Wl,-z,now -Wl,--as-needed -pie'
endif
ifeq ($(UNAME_S),Darwin)
    NGINX_SRC += nginx-darwin
    NGX_OPT= $(NGX_MODULES)
endif
DOCKER_BUILD=./docker
DOCKER_MODULE_IMAGE = $(DOCKER_REPO)/${MODULE_NAME}
DOCKER_MODULE_BASE_IMAGE = $(DOCKER_REPO)/${MODULE_NAME}-base
DOCKER_RUST_IMAGE = $(DOCKER_REPO)/ngx-rust-tool:${RUST_COMPILER_TAG}
DOCKER_NGIX_IMAGE = $(DOCKER_REPO)/nginx-dev:${NGINX_VER}
MODULE_SO_DIR=nginx/nginx-linux/objs
MODULE_SO_BIN=${MODULE_SO_DIR}/${MODULE_NAME}.so
MODULE_SO_HOST=module/release/${MODULE_NAME}.so
DOCKER_BUILD_TOOL=docker run -it --rm -v ${ROOT_DIR}:/src -w /src/${MODULE_PROJ_NAME} ${DOCKER_RUST_IMAGE}
DOCKER_NGINX_TOOL=docker run -it --rm -v ${ROOT_DIR}:/src -w /src/${MODULE_PROJ_NAME} ${DOCKER_NGIX_IMAGE}
DOCKER_NGINX_NAME=nginx-test
DOCKER_NGINX_EXEC=docker exec -it ${DOCKER_NGINX_NAME}
DOCKER_NGINX_EXECD=docker exec -d ${DOCKER_NGINX_NAME}
DOCKER_NGINX_DAEMON=docker run -d -p 8000:8000  --privileged --name  ${DOCKER_NGINX_NAME} \
	-v ${MODULE_DIR}/config/modules:/etc/nginx/modules \
	-v ${MODULE_DIR}:/src  -w /src   ${DOCKER_NGIX_IMAGE}


setup-nginx:
	mkdir -p nginx


nginx-source:	setup-nginx
	rm -rf nginx/${NGINX_SRC}
	wget http://nginx.org/download/nginx-${NGINX_VER}.tar.gz
	tar zxf nginx-${NGINX_VER}.tar.gz
	mv nginx-${NGINX_VER} ${NGINX_SRC}
	mv ${NGINX_SRC} nginx
	rm nginx-${NGINX_VER}.tar.gz*

nginx-configure:
	cd nginx/${NGINX_SRC}; \
    ./configure --add-dynamic-module=../../module $(NGX_OPT)


nginx-setup:	nginx-source nginx-configure


nginx-module:
	cd nginx/${NGINX_SRC}; \
	make modules;  \
	strip objs/*.so



copy-module:
	docker rm -v ngx-copy || true
	docker create --name ngx-copy ${DOCKER_MODULE_IMAGE}:latest
	docker cp ngx-copy:/src/${MODULE_SO_BIN} ${MODULE_SO_HOST}
	docker rm -v ngx-copy

# build module using docker
# we copy only necessary context to docker daemon (src and module directory)
build-module-docker:
	docker build -f $(DOCKER_BUILD)/context/Dockerfile.module -t ${DOCKER_MODULE_IMAGE}:latest .

# build module and deposit in the module directory
build-module: build-module-docker copy-module

# build base container image that pre-compiles rust and nginx modules
build-base:
	docker build -f $(DOCKER_BUILD)/Dockerfile.base -t ${DOCKER_MODULE_BASE_IMAGE}:${GIT_COMMIT} .
	docker tag ${DOCKER_MODULE_BASE_IMAGE}:${GIT_COMMIT} ${DOCKER_MODULE_BASE_IMAGE}:latest


# setup nginx container for testing
# copies the configuration and modules
# start test services
test-nginx-setup:
	cp config/nginx.conf /etc/nginx
	rm -rf /etc/nginx/conf.d/*
	cp config/conf.d/* /etc/nginx/conf.d
	node tests/services/http.js 9100 > u1.log 2> u1.err &
	node tests/services/tcp-invoke.js 9000 dest > t1.log 2> t1.err
	tests/prepare_proxy.sh -p 15001 -u ${DOCKER_USER} &
	nginx -s reload


# remove nginx container
test-nginx-clean:
	docker rm -f  ${DOCKER_NGINX_NAME} || true


test-nginx-only: test-nginx-clean
	$(DOCKER_NGINX_DAEMON)
	$(DOCKER_NGINX_EXECD) make test-nginx-setup > make.out
	sleep 1


test-nginx-log:
	docker logs -f nginx-test


test-nginx-full:	build-module test-nginx-only

# invoke http service
test-tcp:
	nc localhost 8000
