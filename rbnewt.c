#include <ruby.h>
#include <stomp/stomp.h>

VALUE func_stomp_close(VALUE _self) {
  return Qnil;
}

static VALUE stomp_new_alloc(VALUE klass) {
  stomp_session_t *session;

  return rb_data_object_alloc(klass, session, 0, NULL);
}

void Init_rbnewt(void) {
  VALUE module, stomp_kls;

  module = rb_define_module("Newt");
  stomp_kls = rb_define_class_under(module, "STOMP", rb_cObject);

  rb_define_alloc_func(stomp_kls, stomp_new_alloc);
  rb_define_method(stomp_kls, "close", func_stomp_close, 0);
}
