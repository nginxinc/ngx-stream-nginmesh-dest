MODULE_NAME=ngx_stream_nginmesh_dest_module
MODULE_PROJ_NAME=ngx-stream-nginmesh-dest
NGX_DEBUG="--with-debug"
include nginx.mk


clean:
	rm -rf docker/context
	rm -rf nginx
