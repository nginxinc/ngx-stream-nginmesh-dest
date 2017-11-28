MODULE_NAME=ngx_stream_nginmesh_dest_module
MODULE_PROJ_NAME=ngx-stream-nginmesh-dest
NGX_DEBUG="--with-debug"
VERSION=0.2.12-RC2
REPO=ngx-stream-nginmesh-dest
USER=nginmesh

include nginx.mk


clean:
	rm -rf docker/context
	rm -rf nginx



release:	create_release upload_release	

create_release:
	github-release release \
    --user ${USER} \
    --repo ${REPO} \
    --tag ${VERSION} \
    --name "${VERSION}" \
    --pre-release


upload_release:
	github-release upload \
    	--user ${USER} \
    	--repo ${REPO} \
    	--tag ${VERSION} \
    	--name "${MODULE_NAME}.so" \
    	--file module/release/${MODULE_NAME}.so


delete_release:
	github-release delete \
    --user ${USER} \
    --repo ${REPO} \
    --tag ${VERSION}