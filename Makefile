MODULE_NAME=ngx_ngin_mesh_module
MODULE_PROJ_NAME=ngx-stream-nginmesh-dest
NGX_DEBUG="--with-debug"
include nginx.mk


clean:
	cargo clean



super_clean: clean
	rm -rf nginx/*

test:
	cargo test -- --nocapture
