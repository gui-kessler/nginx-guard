# Nginx Guard — Milestone 01

Projeto de estudo: modulo HTTP dinamico do Nginx para mitigacoes centralizadas em aplicacoes web on-premise.

## O que esta implementado

- Modulo nativo em C, compilado como `ngx_http_guard_module.so`.
- Registro de handler em `NGX_HTTP_PREACCESS_PHASE`.
- Bloqueio **fixo** de `POST /api/admin` com HTTP 403.
- Passagem de outras requisicoes usando `NGX_DECLINED`.
- Nenhuma alteracao em `location` e nenhum proxy adicional.

**Nao e um WAF de producao.** Esta versao nao carrega politicas remotas, nao tem rate limit, autenticacao, telemetria, rollback, observabilidade nem hardening.

## Iniciar laboratorio isolado (Docker)

Na raiz do projeto:

```bash
docker build --build-arg NGINX_VERSION=1.28.0 -t nginx-guard:lab .
docker run --rm --name nginx-guard -p 127.0.0.1:8080:8080 nginx-guard:lab
```

Em outro terminal:

```bash
curl -i http://127.0.0.1:8080/health.txt            # 200
curl -i http://127.0.0.1:8080/api/admin             # 200
curl -i -X POST http://127.0.0.1:8080/api/admin     # 403
./tests/smoke.sh
```

O build do laboratorio compila nginx e modulo a partir da **mesma** fonte e flags. Isso evita divergencia de assinatura binaria **dentro do laboratorio**. A versao 1.28.0 e apenas a versao de exemplo do sandbox, NAO uma afirmacao sobre a versao usada na empresa.

Se quiser usar outra versao, informe `NGINX_VERSION` ao construir a imagem. A versao precisa existir no repositório oficial de fontes do Nginx.

## Estrutura

```
module/config                    Declaracao de modulo para o sistema de build Nginx
module/ngx_http_guard_module.c   Handler HTTP e registro da fase
sandbox/nginx.conf               Config isolada com load_module global
sandbox/site/                   Arquivos para testes HTTP estaticos
tests/smoke.sh                  Testes basicos de comportamento
Dockerfile                      Build isolado do modulo e do Nginx
```

## O que estudar no codigo

1. `ngx_module_t`: define o modulo para o Nginx.
2. `ngx_http_module_t`: define os hooks de configuracao HTTP.
3. `ngx_http_guard_init`: registra o handler na fase PREACCESS.
4. `ngx_http_guard_handler`: avalia metodo/URI e retorna `NGX_DECLINED` ou `NGX_HTTP_FORBIDDEN`.
5. `r->uri`: URI normalizada, sem query string; a politica de canonicalizacao precisara ser definida futuramente.

## Limitacao importante da fase

`PREACCESS` nao equivale a 'toda requisicao HTTP sem excecao'. Algumas configuracoes, como `return` executado na fase `rewrite`, podem finalizar a requisicao antes de ela chegar ao handler. Numa evolucao, testaremos multiplas fases e potenciais bypasses, alem de redirecionamentos internos, subrequests e interpretacoes de URI.

## Compatibilidade em instalacoes reais

Mesma versao do Nginx nao garante, sozinha, compatibilidade de modulo dinamico. Tambem importam flags de compilacao, build/ABI, arquitetura e origem do pacote.

No servidor de laboratorio representativo, capture:

```bash
nginx -v
nginx -V 2>&1
```

Nao instale este `.so` experimental em servidores de clientes. Primeiro, construa um laboratorio com o binario e o conjunto de flags reais e verifique `nginx -t`. A instalacao de um modulo novo exige incluir `load_module` no contexto principal e recarregar a configuracao uma vez; **atualizacao de politicas sem reload** sera um marco futuro, separado.

## Roadmap

- [x] M01 — Interceptar e bloquear rota fixa.
- [ ] M02 — Formato de politica local (rota/metodo, host, id, enabled, expira em).
- [ ] M03 — Publicacao atomica e recarga de politica sem `nginx -s reload`.
- [ ] M04 — Rate limit com estado compartilhado entre workers.
- [ ] M05 — Integração com agente existente (autenticacao, assinatura, versionamento, rollback).
- [ ] M06 — Canary rollout, compatibilidade, fuzzing de URI e testes de bypass.

## Referencias oficiais

- https://nginx.org/en/docs/dev/development_guide.html
- https://nginx.org/en/docs/configure.html
- https://docs.nginx.com/nginx/admin-guide/installing-nginx/installing-nginx-open-source/
