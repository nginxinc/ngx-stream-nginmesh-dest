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

typedef struct {
    ngx_flag_t      enabled;
} ngx_stream_nginmesh_srv_conf_t;

typedef struct {
    ngx_str_t       dest;
    ngx_str_t       port;
    ngx_pool_t      *pool;
} ngx_stream_nginmesh_ctx_t;



static ngx_int_t ngx_stream_nginmesh_handler(ngx_stream_session_t *s);
static ngx_int_t ngx_stream_nginmesh_dest_variable(ngx_stream_session_t *s,ngx_stream_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_stream_ngin_add_variables(ngx_conf_t *cf);
static ngx_int_t ngx_stream_ngin_mesh_init(ngx_conf_t *cf);
static void *ngx_stream_nginmesh_create_srv_conf(ngx_conf_t *cf);
static char *ngx_stream_nginmesh_merge_srv_conf(ngx_conf_t *cf, void *parent, void *child);



/**
 * This module provide callback to istio for http traffic
 *
 */
static ngx_command_t ngx_stream_mesh_commands[] = {

    {
      ngx_string("nginmesh_dest"),
      NGX_STREAM_MAIN_CONF | NGX_STREAM_SRV_CONF | NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,     // do custom config
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_stream_nginmesh_srv_conf_t, enabled),
      NULL
    },
    ngx_null_command /* command termination */
};




/* The module context. */
static ngx_stream_module_t ngx_stream_mesh_module_ctx = {
    ngx_stream_ngin_add_variables, /* preconfiguration */
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

// list of variables to add
static ngx_stream_variable_t  ngx_stream_nginmesh_vars[] = {

    { ngx_string("nginmesh_dest"), NULL,
      ngx_stream_nginmesh_dest_variable, 0, 0, 0 },
    ngx_stream_null_variable
};



static void *ngx_stream_nginmesh_create_srv_conf(ngx_conf_t *cf)
{
    ngx_stream_nginmesh_srv_conf_t  *conf;

    ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "nginmesh create serv config");

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_stream_nginmesh_srv_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->enabled = NGX_CONF_UNSET;

    return conf;
}


static char *ngx_stream_nginmesh_merge_srv_conf(ngx_conf_t *cf, void *parent, void *child)
{

    ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "nginmesh merge serv config");

    ngx_stream_nginmesh_srv_conf_t *prev = parent;
    ngx_stream_nginmesh_srv_conf_t *conf = child;

    ngx_conf_merge_value(conf->enabled, prev->enabled, 0);

    return NGX_CONF_OK;
}




static ngx_int_t ngx_stream_nginmesh_handler(ngx_stream_session_t *s)
{

    ngx_stream_nginmesh_srv_conf_t      *meshcf;
    struct sockaddr_storage             org_src_addr;
    socklen_t                           org_src_addr_len;
    ngx_connection_t                    *c;
    ngx_stream_nginmesh_ctx_t           *ctx;
    char                                dest_text[30];  // 4*4 + 5 + 1



    c = s->connection;

	ngx_log_debug(NGX_LOG_DEBUG_STREAM,  s->connection->log, 0,"nginmesh stream handler invoked");


    meshcf = ngx_stream_get_module_srv_conf(s, ngx_ngin_mesh_module);

    if (!meshcf->enabled) {
        ngx_log_debug(NGX_LOG_DEBUG_STREAM, s->connection->log, 0, "nginmesh not enabled, declined");
        return NGX_DECLINED;
    }

    if (c->type != SOCK_STREAM) {
        ngx_log_debug(NGX_LOG_DEBUG_STREAM, s->connection->log, 0, "nginmesh not sock stream  declined");
        return NGX_DECLINED;
    }

    if (c->buffer == NULL) {
        ngx_log_debug(NGX_LOG_DEBUG_STREAM, s->connection->log, 0, "nginmesh no buffer, again");
        return NGX_AGAIN;
    }

    ctx = ngx_stream_get_module_ctx(s, ngx_ngin_mesh_module);

    if (ctx == NULL) {
         ngx_log_debug(NGX_LOG_DEBUG_STREAM, s->connection->log, 0, "nginmesh creating new context");
        ctx = ngx_pcalloc(c->pool, sizeof(ngx_stream_nginmesh_ctx_t));
        if (ctx == NULL) {
            return NGX_ERROR;
        }

        ngx_stream_set_ctx(s, ctx, ngx_ngin_mesh_module);

        ctx->pool = c->pool;
    }


  //  len = ngx_sock_ntop(c->sockaddr, sizeof(struct sockaddr_in),text,c->socklen,1);
   ngx_log_debug2(NGX_LOG_DEBUG_STREAM, s->connection->log, 0, "module original ip address: %*s",c->addr_text.len,c->addr_text.data);


    ngx_memzero(&org_src_addr, sizeof(struct sockaddr));
     org_src_addr_len =  sizeof(struct sockaddr);
    if(getsockopt ( c->fd, SOL_IP, SO_ORIGINAL_DST, &org_src_addr,&org_src_addr_len) == -1) {
         ngx_log_error(NGX_LOG_ALERT, s->connection->log, ngx_socket_errno,
                                      "failed to get original ip address");
         return NGX_DECLINED;

    } else {

        ngx_log_debug1(NGX_LOG_DEBUG_STREAM, s->connection->log,0, "ip address length %d",org_src_addr_len);

        if(org_src_addr.ss_family == AF_INET )  {
           ngx_log_debug(NGX_LOG_DEBUG_STREAM, s->connection->log, 0, "address is is INET format");
           struct sockaddr_in *addr_in = (struct sockaddr_in *)&org_src_addr;
           char *s = inet_ntoa(addr_in->sin_addr);
           int port = ntohs(addr_in->sin_port);
           ngx_log_debug1(NGX_LOG_DEBUG_STREAM, s->connection->log, 0, "founded dest ip address: %s",s);
           ngx_log_debug1(NGX_LOG_DEBUG_STREAM, s->connection->log,  0, "founded dest port address: %d",port);

           ngx_memzero(dest_text,30);
           sprintf(dest_text,"%s:%d",s,port);
           ngx_log_debug1(NGX_LOG_DEBUG_STREAM, s->connection->log,  0, "combined text: %s",dest_text);
           size_t dest_str_size = ngx_strlen(dest_text);


           ctx->dest.data = ngx_pnalloc(ctx->pool, dest_str_size);
           ctx->dest.len = dest_str_size;
           ngx_memcpy(ctx->dest.data,dest_text,dest_str_size);

           ngx_log_debug2(NGX_LOG_DEBUG_STREAM, s->connection->log,  0, "ctx dest  var %*s",ctx->dest.len,ctx->dest.data);


           
        } else {

           ngx_log_debug(NGX_LOG_DEBUG_STREAM, s->connection->log,  0, "address is is not INET format");
           return NGX_DECLINED;
        }


    }

    return NGX_OK;

}

// assign variable from ctx
static ngx_int_t ngx_stream_nginmesh_dest_variable(ngx_stream_session_t *s,
    ngx_variable_value_t *v, uintptr_t data)
{
    ngx_stream_nginmesh_ctx_t  *ctx;

    ctx = ngx_stream_get_module_ctx(s, ngx_ngin_mesh_module);

    if (ctx == NULL) {
        v->not_found = 1;
        return NGX_OK;
    }

    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->len = ctx->dest.len;
    v->data = ctx->dest.data;

	ngx_log_debug2(NGX_LOG_DEBUG_HTTP,  s->connection->log, 0,"set var nginmesh_dest %*s",v->len,v->data);

    return NGX_OK;
}


static ngx_int_t ngx_stream_ngin_add_variables(ngx_conf_t *cf)
{
    ngx_stream_variable_t  *var, *v;


    for (v = ngx_stream_nginmesh_vars; v->name.len; v++) {
        ngx_log_debug2(NGX_LOG_DEBUG_EVENT, ngx_cycle->log, 0, "ngin mesh var initialized: %*s",v->name.len,v->name.data);
        var = ngx_stream_add_variable(cf, &v->name, v->flags);
        if (var == NULL) {
            return NGX_ERROR;
        }

        var->get_handler = v->get_handler;
        var->data = v->data;
    }

    return NGX_OK;
}

// add handler to pre-access
// otherwise, handler can't be add as part of config handler if proxy handler is involved.

static ngx_int_t ngx_stream_ngin_mesh_init(ngx_conf_t *cf)
{
    ngx_stream_handler_pt        *h;
    ngx_stream_core_main_conf_t  *cmcf;


    ngx_log_debug(NGX_LOG_DEBUG_EVENT,  ngx_cycle->log, 0, "ngin mesh init invoked");


    cmcf = ngx_stream_conf_get_module_main_conf(cf, ngx_stream_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_STREAM_PREREAD_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_stream_nginmesh_handler;

    return NGX_OK;
}

