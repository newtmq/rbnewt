#include <ruby/ruby.h>
#include <ruby/intern.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stomp/stomp.h>

#include <newt/stomp.h>
#include <newt/rbnewt.h>

static VALUE get_headers(struct stomp_ctx_message *ctx) {
  VALUE ret = rb_hash_new();
  const struct stomp_hdr *hdrs;
  int i;

  assert(ctx != NULL);

  hdrs = ctx->hdrs;
  for(i=0; i<ctx->hdrc; i++) {
    rb_hash_aset(ret, rb_str_new2(hdrs[i].key), rb_str_new2(hdrs[i].val));
  }

  return ret;
}

static VALUE get_body(struct stomp_ctx_message *ctx) {
  VALUE ret = Qnil;

  assert(ctx != NULL);

  if(ctx->body != NULL && ctx->body_len > 0) {
    ret = rb_str_new2((char *)ctx->body);
  }

  return ret;
}

static struct stomp_hdr *make_stomp_hdr(VALUE rb_opts, int offset, int *header_len) {
  struct stomp_hdr *headers = NULL;
  int i;
  VALUE rb_headers = Qnil;

  assert(header_len != NULL);

  *header_len = offset;

  if(! NIL_P(rb_opts)) {
    assert(TYPE(rb_opts) == T_HASH);

    rb_headers = rb_funcall(rb_opts, rb_intern("to_a"), 0);

    assert(TYPE(rb_headers) == T_ARRAY);

    *header_len += RARRAY_LEN(rb_headers);
  }

  headers = (struct stomp_hdr*)malloc(sizeof(struct stomp_hdr) * (*header_len));

  for(i=offset; i<*header_len; i++) {
    assert(TYPE(rb_headers) == T_ARRAY);

    VALUE entry = rb_ary_entry(rb_headers, i);

    VALUE key = rb_ary_entry(entry, 0);
    VALUE val = rb_ary_entry(entry, 1);

    headers[i].key = StringValueCStr(key);
    headers[i].val = StringValueCStr(val);
  }

  return headers;
}

static void do_newt_stomp_callback(stomp_session_t *s, void *callback_ctx, char *cb_method_name) {
  rbnewt_context_t *rbnewt_ctx = (rbnewt_context_t *)s->ctx;

  assert(rbnewt_ctx != NULL);
  assert(cb_method_name != NULL);

  if(rb_respond_to(rbnewt_ctx->callback_obj, rb_intern(cb_method_name))) {
    VALUE header = get_headers((struct stomp_ctx_message *)callback_ctx);
    VALUE body = get_body((struct stomp_ctx_message *)callback_ctx);

    VALUE ret = rb_funcall(rbnewt_ctx->callback_obj, rb_intern(cb_method_name), 2, header, body);
    if(TYPE(ret) == T_FALSE) {
      s->run = 0;
    }
  }
}
void newt_stomp_callback_connected(stomp_session_t *s, void *callback_ctx, void *session_ctx) {
  do_newt_stomp_callback(s, callback_ctx, NEWT_STOMP_CB_CONNECTED);
}
void newt_stomp_callback_message(stomp_session_t *s, void *callback_ctx, void *session_ctx) {
  do_newt_stomp_callback(s, callback_ctx, NEWT_STOMP_CB_MESSAGE);
}
void newt_stomp_callback_error(stomp_session_t *s, void *callback_ctx, void *session_ctx) {
  do_newt_stomp_callback(s, callback_ctx, NEWT_STOMP_CB_ERROR);
}

static char *get_str_from_hash(VALUE hash, char *key) {
  assert(TYPE(hash) == T_HASH);
  
  VALUE v = rb_hash_aref(hash, ID2SYM(rb_intern(key)));
  if(v != Qnil && TYPE(v) == T_STRING) {
    return StringValueCStr(v);
  }

  return NULL;
}

static int get_int_from_hash(VALUE hash, char *key) {
  assert(TYPE(hash) == T_HASH);

  VALUE v = rb_hash_aref(hash, ID2SYM(rb_intern(key)));
  if(v != Qnil && TYPE(v) == T_FIXNUM) {
    return NUM2INT(v);
  }

  return -1;
}

static VALUE newt_stomp_initialize(int argc, VALUE *argv, VALUE self) {
  stomp_session_t *session;
  VALUE cbobj, opts;
  char *server, *userid, *passwd, *port;
  rbnewt_context_t *context;

  rb_scan_args(argc, argv, "02", &cbobj, &opts);

  Data_Get_Struct(self, stomp_session_t, session);

  // zero-set for initialization
  server = userid = passwd = port = NULL;

  if(! NIL_P(opts)) {
    server = get_str_from_hash(opts, NEWT_STOMP_KEY_SERVER);
    userid = get_str_from_hash(opts, NEWT_STOMP_KEY_USERID);
    passwd = get_str_from_hash(opts, NEWT_STOMP_KEY_PASSWD);
    port   = get_str_from_hash(opts, NEWT_STOMP_KEY_PORT);
  }

  // set default values
  if(server == NULL) server = NEWT_STOMP_DEFAULT_SERVER;
  if(userid == NULL) userid = NEWT_STOMP_DEFAULT_USERID;
  if(passwd == NULL) passwd = NEWT_STOMP_DEFAULT_PASSWD;
  if(port == NULL)   port   = NEWT_STOMP_DEFAULT_PORT;

  // set self object to context
  context = (rbnewt_context_t *)session->ctx;
  context->callback_obj = cbobj;

  if(cnewt_stomp_initialize(session, server, port, userid, passwd) != RET_SUCCESS) {
    printf("[warning] failed to connect server\n");
  }

  return self;
}

static VALUE newt_stomp_publish(int argc, VALUE *argv, VALUE self) {
  struct stomp_hdr *headers;
  stomp_session_t *session;
  VALUE rb_dest, rb_data, rb_opts;
  int header_len;

  // initialize each rb values
  rb_scan_args(argc, argv, "21", &rb_dest, &rb_data, &rb_opts);

  Data_Get_Struct(self, stomp_session_t, session);

  char *body = StringValuePtr(rb_data);
  // making headers
  headers = make_stomp_hdr(rb_opts, 1, &header_len);
  headers[0].key = "destination";
  headers[0].val = StringValuePtr(rb_dest);

  stomp_send(session, header_len, headers, body, strlen(body));

  free(headers);

  return self;
}

static VALUE newt_stomp_subscribe(int argc, VALUE *argv, VALUE self) {
  struct stomp_hdr *headers;
  stomp_session_t *session;
  VALUE rb_dest, rb_opts;
  int header_len;

  // initialize each rb values
  rb_scan_args(argc, argv, "11", &rb_dest, &rb_opts);

  Data_Get_Struct(self, stomp_session_t, session);

  // making headers
  headers = make_stomp_hdr(rb_opts, 1, &header_len);
  headers[0].key = "destination";
  headers[0].val = StringValuePtr(rb_dest);

  stomp_subscribe(session, header_len, headers);

  free(headers);

  return self;
}

static VALUE newt_stomp_run(VALUE self) {
  stomp_session_t *session;
  rbnewt_context_t *context;

  Data_Get_Struct(self, stomp_session_t, session);

  cnewt_stomp_start(session);

  return self;
}

static void context_free(void *ptr) {
  stomp_session_t *session = (stomp_session_t *)ptr;

  stomp_session_free(session);
}

static VALUE newt_stomp_alloc(VALUE klass) {
  stomp_session_t *session;
  rbnewt_context_t *context;
  VALUE ret = Qnil;

  context = (rbnewt_context_t *)malloc(sizeof(rbnewt_context_t));
  assert(context != NULL);

  session = stomp_session_new(context);
  if(session != NULL) {
    ret = rb_data_object_alloc(klass, session, 0, context_free);
  }

  return ret;
}

void Init_rbnewt(void) {
  VALUE module, stomp_kls;

  module = rb_define_module("Newt");
  stomp_kls = rb_define_class_under(module, "STOMP", rb_cObject);

  rb_define_alloc_func(stomp_kls, newt_stomp_alloc);
  rb_define_method(stomp_kls, "initialize", newt_stomp_initialize, -1);
  rb_define_method(stomp_kls, "publish", newt_stomp_publish, -1);
  rb_define_method(stomp_kls, "subscribe", newt_stomp_subscribe, -1);
  rb_define_method(stomp_kls, "run", newt_stomp_run, 0);
}
