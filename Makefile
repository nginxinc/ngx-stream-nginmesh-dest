MODULE_NAME=ngx_stream_nginmesh_dest_module
MODULE_PROJ_NAME=ngx-stream-nginmesh-dest
NGX_DEBUG="--with-debug"
include nginx.mk


clean:
	cargo clean



super_clean: clean
	rm -rf module/*.so
	rm -rf build/crate
	rm -rf build/context
	rm -rf nginx

test:
	cargo test -- --nocapture
