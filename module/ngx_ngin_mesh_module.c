/**
 * @file   ngx_http_istio_mixer_module.c
 * @author Sehyo Chang <sehyo@nginx.com>
 * @date   Wed Aug 19 2017
 *
 * @brief  Istio Mixer integration  module for Nginx.
 *
 * @section LICENSE
 *
 * Copyright (C) 2011 by Nginx
 *
 */
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_stream.h>
#include <sys/socket.h>
#include <linux/netfilter_ipv4.h>


typedef struct  {

} ngx_stream_nginmesh_srv_conf_t;



static void *ngx_stream_nginmesh_create_srv_conf(ngx_conf_t *cf);
static char *ngx_stream_nginmesh_merge_srv_conf(ngx_conf_t *cf, void *parent, void *child);
static char * ngx_ngin_mesh_config_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_stream_nginmesh_handler(ngx_stream_session_t *s);
static ngx_int_t ngx_stream_ngin_mesh_init(ngx_conf_t *cf);


/**
 * This module provide callback to istio for http traffic
 *
 */
static ngx_command_t ngx_stream_mesh_commands[] = {

    {
      ngx_string("set_ngin_mesh_route"),
      NGX_STREAM_SRV_CONF | NGX_CONF_TAKE1,
      ngx_ngin_mesh_config_handler,     // do custom config
      NGX_STREAM_SRV_CONF_OFFSET,
      0,
      NULL
    },
    ngx_null_command /* command termination */
};




/* The module context. */
static ngx_stream_module_t ngx_stream_mesh_module_ctx = {
    NULL, /* preconfiguration */
    ngx_stream_ngin_mesh_init, /* postconfiguration */
    NULL,
    NULL, /* init main configuration */
    ngx_stream_nginmesh_create_srv_conf, /* create server configuration */
    ngx_stream_nginmesh_merge_srv_conf /* merge server configuration */

};

/* Module definition. */
ngx_module_t ngx_ngin_mesh_module = {
    NGX_MODULE_V1,
    &ngx_stream_mesh_module_ctx, /* module context */
    ngx_stream_mesh_commands, /* module directives */
    NGX_STREAM_MODULE, /* module type */
    NULL, /* init master */
    NULL, /* init module */
    NULL, /* init process */
    NULL, /* init thread */
    NULL, /* exit thread */
    NULL, /* exit process */
    NULL, /* exit master */
    NGX_MODULE_V1_PADDING
};

static void *ngx_stream_nginmesh_create_srv_conf(ngx_conf_t *cf)
{
    ngx_stream_nginmesh_srv_conf_t  *conf;

    ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "ngin mesh create srv conf");


    conf = ngx_pcalloc(cf->pool, sizeof(ngx_stream_nginmesh_srv_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    return conf;
}


static char *ngx_stream_nginmesh_merge_srv_conf(ngx_conf_t *cf, void *parent, void *child)
{
    //ngx_stream_nginmesh_srv_conf_t *prev = parent;
    // ngx_stream_nginmesh_srv_conf_t *conf = child;

    ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "ngin mesh merge srv conf");


    return NGX_CONF_OK;
}




// handle configuration
static char * ngx_ngin_mesh_config_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {


 //   ngx_stream_nginmesh_srv_conf_t *pscf = conf;
    /*
    ngx_stream_core_srv_conf_t          *cscf;

    ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "ngin mesh config handler invoked");


    cscf = ngx_stream_conf_get_module_srv_conf(cf, ngx_stream_core_module);

    //cscf->handler = ngx_stream_nginmesh_handler;
        */

     return NGX_CONF_OK;

}


static ngx_int_t ngx_stream_nginmesh_handler(ngx_stream_session_t *s)
{

    const char               *val;
    struct sockaddr_storage    org_src_addr;
    socklen_t  org_src_addr_len;
    ngx_connection_t                 *c;


    //ngx_stream_nginmesh_srv_conf_t  *nscf;

   // nscf = ngx_stream_get_module_srv_conf(s, ngx_ngin_mesh_module);

    ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "ngin mesh handler invoked");

    c = s->connection;

  //  len = ngx_sock_ntop(c->sockaddr, sizeof(struct sockaddr_in),text,c->socklen,1);
    ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "module original ip address: %*s",c->addr_text.len,c->addr_text.data);


    ngx_memzero(&org_src_addr, sizeof(struct sockaddr));
     org_src_addr_len =  sizeof(struct sockaddr);
    if(getsockopt ( c->fd, SOL_IP, SO_ORIGINAL_DST, &org_src_addr,&org_src_addr_len) == -1) {
         ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, ngx_socket_errno,
                                      "failed to get original ip address");

    } else {

        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "ip address length %d",org_src_addr_len);

        if(org_src_addr.ss_family == AF_INET )  {
               ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "address is is INET format");
               struct sockaddr_in *addr_in = (struct sockaddr_in *)&org_src_addr;
               char *s = inet_ntoa(addr_in->sin_addr);
            //  len = ngx_sock_ntop((struct sockaddr *)&org_src_addr,org_src_addr_len,text,NGX_SOCKADDR_STRLEN,1);
              ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "founded original ip address: %s",s);
        } else {

             ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "address is is not INET format");
        }


    }

    return NGX_OK;

}

// add handler to pre-access
// otherwise, handler can't be add as part of config handler if proxy handler is involved.

static ngx_int_t ngx_stream_ngin_mesh_init(ngx_conf_t *cf)
{
    ngx_stream_handler_pt        *h;
    ngx_stream_core_main_conf_t  *cmcf;

     ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "ngin mesh init invoked");


    cmcf = ngx_stream_conf_get_module_main_conf(cf, ngx_stream_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_STREAM_PREACCESS_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_stream_nginmesh_handler;

    return NGX_OK;
}



