#include <ruby/ruby.h>
#include <ruby/intern.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <stomp/connection.h>
#include <stomp/common.h>
#include <stomp/stomp.h>
#include <stomp/frame.h>

#include <newt/stomp_manager.h>
#include <newt/message.h>
#include <newt/rbnewt.h>

#define DEFAULT_CONCURRENCY 1

struct stomp_hdr {
  char data[LD_MAX];
  int len;
};

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

    headers[i].len = stpring(headers[i].data, "%s:%s\n", StringValueCStr(key), StringValueCStr(val));
  }

  return headers;
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
  stomp_manager_t *manager;
  VALUE opts;
  char *server, *userid, *passwd;
  int port = 0;

  rb_scan_args(argc, argv, "01", &opts);

  Data_Get_Struct(self, stomp_manager_t, manager);

  // zero-set for initialization
  server = userid = passwd = NULL;

  if(! NIL_P(opts)) {
    server = get_str_from_hash(opts, NEWT_STOMP_KEY_SERVER);
    userid = get_str_from_hash(opts, NEWT_STOMP_KEY_USERID);
    passwd = get_str_from_hash(opts, NEWT_STOMP_KEY_PASSWD);
    port   = get_int_from_hash(opts, NEWT_STOMP_KEY_PORT);
  }

  // set default values
  if(server == NULL) server = NEWT_STOMP_DEFAULT_SERVER;
  if(userid == NULL) userid = NEWT_STOMP_DEFAULT_USERID;
  if(passwd == NULL) passwd = NEWT_STOMP_DEFAULT_PASSWD;
  if(port == 0)      port   = NEWT_STOMP_DEFAULT_PORT;

  if(stomp_worker_start(manager, server, port, userid, passwd) != RET_SUCCESS) {
    rb_raise(rb_eRuntimeError, "failed to connect server");
  }

  /*
  frame_t *frame = stomp_manager_recv(manager);
  if(frame == NULL) {
    rb_raise(rb_eRuntimeError, "failed to receive CONNECTED frame");
  }

  if(frame->cmd_len != 9 || strncmp(frame->cmd, "CONNECTED", 9) != 0) {
    rb_raise(rb_eRuntimeError, "authentication with STOMP server is failed");
  }
  */

  return self;
}

static VALUE newt_stomp_publish(int argc, VALUE *argv, VALUE self) {
  struct stomp_hdr *headers;
  stomp_manager_t *manager;
  VALUE rb_dest, rb_data, rb_opts;
  int header_len;

  // initialize each rb values
  rb_scan_args(argc, argv, "21", &rb_dest, &rb_data, &rb_opts);

  // retrieve stomp_manager_t object
  Data_Get_Struct(self, stomp_manager_t, manager);

  // making headers
  headers = make_stomp_hdr(rb_opts, 1, &header_len);
  headers[0].len = sprintf(headers[0].data, "destination:%s\n", StringValuePtr(rb_dest));

  /*
  // sending processing (send STOMP command)
  conn_send(session->conn, "SEND\n", 5);

  // sending processing (send headres)
  int i;
  for(i=0; i<header_len; i++) {
    conn_send(session->conn, headers[i].data, headers[i].len);
  }
  conn_send(session->conn, "\n", 1);

  // sending processing (send body)
  //char *body = StringValuePtr(rb_data);
  conn_send(session->conn, RSTRING_PTR(rb_data), RSTRING_LEN(rb_data));
  conn_send(session->conn, "\0", 1);
  */

  // make and send frame
  frame_t *frame = frame_init();
  if(frame != NULL) {
    frame_set_cmd(frame, "SEND", 4);

    // set headers to frame
    int i;
    for(i=0; i<header_len; i++) {
      frame_set_header(frame, headers[i].data, headers[i].len);
    }

    // set body data to frame
    char *body = StringValuePtr(rb_data);
    frame_set_body(frame, body, strlen(body));

    stomp_manager_send(manager, frame);
  }

  free(headers);

  return self;
}

static VALUE newt_stomp_subscribe(int argc, VALUE *argv, VALUE self) {
  struct stomp_hdr *headers;
  stomp_manager_t *manager;
  VALUE rb_dest, rb_opts, ret = Qfalse;
  frame_t *frame;
  int header_len;

  // initialize each rb values
  rb_scan_args(argc, argv, "11", &rb_dest, &rb_opts);

  Data_Get_Struct(self, stomp_manager_t, manager);

  // making headers
  headers = make_stomp_hdr(rb_opts, 1, &header_len);
  headers[0].len = sprintf(headers[0].data, "destination:%s\n", StringValuePtr(rb_dest));

  // make and send frame
  frame = frame_init();
  if(frame != NULL) {
    frame_set_cmd(frame, "SUBSCRIBE", 9);

    // set headers to frame
    int i;
    for(i=0; i<header_len; i++) {
      frame_set_header(frame, headers[i].data, headers[i].len);
    }

    stomp_manager_send(manager, frame);

    ret = Qtrue;
  }

  free(headers);

  return ret;
}

static void context_free(void *ptr) {
  stomp_manager_t *manager = (stomp_manager_t *)ptr;

  stomp_worker_stop(manager);
  stomp_manager_cleanup(manager);
}

static VALUE newt_stomp_alloc(VALUE klass) {
  stomp_manager_t *manager;
  VALUE ret = Qnil;

  manager = stomp_manager_init(DEFAULT_CONCURRENCY);
  if(manager != NULL) {
    ret = rb_data_object_alloc(klass, manager, 0, context_free);
  }

  return ret;
}

static VALUE newt_stomp_receive(VALUE self) {
  stomp_manager_t *manager;
  frame_t *frame;

  Data_Get_Struct(self, stomp_manager_t, manager);

  do {
    frame = stomp_manager_recv(manager);
  } while(frame == NULL);

  return alloc_message(frame);
}

void Init_rbnewt(void) {
  VALUE module, stomp_kls;

  module = rb_define_module("Newt");
  stomp_kls = rb_define_class_under(module, "STOMP", rb_cObject);

  init_klass_message(module);

  rb_define_alloc_func(stomp_kls, newt_stomp_alloc);
  rb_define_method(stomp_kls, "initialize", newt_stomp_initialize, -1);
  rb_define_method(stomp_kls, "publish", newt_stomp_publish, -1);
  rb_define_method(stomp_kls, "subscribe", newt_stomp_subscribe, -1);
  rb_define_method(stomp_kls, "receive", newt_stomp_receive, 0);
}
