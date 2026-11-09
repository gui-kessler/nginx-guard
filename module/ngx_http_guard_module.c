#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

/*
 * Milestone 01: uma unica politica fixa para entender a API de fases do Nginx.
 * Proximos passos: regras em memoria, reload sem HUP, sincronizacao com agent.
 */

static ngx_int_t ngx_http_guard_handler(ngx_http_request_t *r);
static ngx_int_t ngx_http_guard_init(ngx_conf_t *cf);

static ngx_http_module_t ngx_http_guard_module_ctx = {
    NULL,                 /* preconfiguration */
    ngx_http_guard_init,  /* postconfiguration */

    NULL,                 /* create main configuration */
    NULL,                 /* init main configuration */

    NULL,                 /* create server configuration */
    NULL,                 /* merge server configuration */

    NULL,                 /* create location configuration */
    NULL                  /* merge location configuration */
};

ngx_module_t ngx_http_guard_module = {
    NGX_MODULE_V1,
    &ngx_http_guard_module_ctx,
    NULL,                 /* commands */
    NGX_HTTP_MODULE,
    NULL,                 /* init master */
    NULL,                 /* init module */
    NULL,                 /* init process */
    NULL,                 /* init thread */
    NULL,                 /* exit thread */
    NULL,                 /* exit process */
    NULL,                 /* exit master */
    NGX_MODULE_V1_PADDING
};

static ngx_int_t
ngx_http_guard_handler(ngx_http_request_t *r)
{
    static const ngx_str_t blocked_uri = ngx_string("/api/admin");

    /* r->uri representa a URI normalizada pelo Nginx, sem query string. */
    if (r->method != NGX_HTTP_POST ||
        r->uri.len != blocked_uri.len ||
        ngx_memcmp(r->uri.data, blocked_uri.data, blocked_uri.len) != 0)
    {
        return NGX_DECLINED;
    }

    ngx_log_error(NGX_LOG_WARN, r->connection->log, 0,
                  "nginx_guard: blocked method=\"%V\" uri=\"%V\"",
                  &r->method_name, &r->uri);

    return NGX_HTTP_FORBIDDEN;
}

static ngx_int_t
ngx_http_guard_init(ngx_conf_t *cf)
{
    ngx_http_core_main_conf_t *cmcf;
    ngx_http_handler_pt      *handler;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    handler = ngx_array_push(&cmcf->phases[NGX_HTTP_PREACCESS_PHASE].handlers);
    if (handler == NULL) {
        return NGX_ERROR;
    }

    *handler = ngx_http_guard_handler;
    return NGX_OK;
}
