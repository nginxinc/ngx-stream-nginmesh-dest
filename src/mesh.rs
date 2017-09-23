extern crate ngx_rust;


use std::env;
use std::mem;
use std::ptr;
use std::str;
use std::slice;


use ngx_rust::bindings::ngx_http_request_s;
use ngx_rust::bindings::ngx_http_headers_in_t;
use ngx_rust::bindings::ngx_http_headers_out_t;
use ngx_rust::bindings::ngx_cycle_t;
use ngx_rust::bindings::ngx_int_t;
use ngx_rust::bindings::ngx_str_t;
use ngx_rust::bindings::NGX_OK;
use ngx_rust::bindings::ngx_flag_t;
use ngx_rust::nginx_http::list_iterator;
use ngx_rust::nginx_http::log;


#[no_mangle]
pub extern fn mesh(request: &ngx_http_request_s)  {


}
