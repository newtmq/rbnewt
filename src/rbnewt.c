#include <ruby.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <stomp/stomp.h>
#include <newt/stomp.h>

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

VALUE newt_stomp_close(VALUE self) {
  stomp_session_t *session;

  Data_Get_Struct(self, stomp_session_t, session);

  if(cnewt_stomp_close(session) == RET_SUCCESS) {
    return self;
  }

  return Qnil;
}

VALUE newt_stomp_initialize(int argc, VALUE *argv, VALUE self) {
  stomp_session_t *session;
  VALUE opts;
  char *server, *userid, *passwd, *port;

  rb_scan_args(argc, argv, "01", &opts);

  Data_Get_Struct(self, stomp_session_t, session);

  // zero-set for initialization
  server = userid = passwd = port = NULL;

  if(! NIL_P(opts)) {
    server = get_str_from_hash(opts, NEWT_STOMP_KEY_SERVER);
    userid = get_str_from_hash(opts, NEWT_STOMP_KEY_USERID);
    passwd = get_str_from_hash(opts, NEWT_STOMP_KEY_PASSWD);
    port   = get_str_from_hash(opts, NEWT_STOMP_KEY_PORT);
  }

  //set default values
  if(server == NULL) server = NEWT_STOMP_DEFAULT_SERVER;
  if(userid == NULL) userid = NEWT_STOMP_DEFAULT_USERID;
  if(passwd == NULL) passwd = NEWT_STOMP_DEFAULT_PASSWD;
  if(port == NULL)   port   = NEWT_STOMP_DEFAULT_PORT;

  if(cnewt_stomp_initialize(session, server, port, userid, passwd) != RET_SUCCESS) {
    printf("[warning] failed to connect server\n");
  }

  return self;
}

static VALUE newt_stomp_alloc(VALUE klass) {
  stomp_session_t *session;
  pthread_t *thread_id;
  VALUE ret = Qnil;

  thread_id = (pthread_t *)malloc(sizeof(pthread_t));
  assert(thread_id != NULL);

  session = stomp_session_new(thread_id);
  if(session != NULL) {
    ret = rb_data_object_alloc(klass, session, 0, NULL);
  }

  return ret;
}

void Init_rbnewt(void) {
  VALUE module, stomp_kls;

  module = rb_define_module("Newt");
  stomp_kls = rb_define_class_under(module, "STOMP", rb_cObject);

  rb_define_alloc_func(stomp_kls, newt_stomp_alloc);
  rb_define_method(stomp_kls, "initialize", newt_stomp_initialize, -1);
  rb_define_method(stomp_kls, "close", newt_stomp_close, 0);
}
