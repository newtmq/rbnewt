#include <newt/message.h>
#include <ruby/ruby.h>

#include <stomp/frame.h>
#include <stomp/list.h>

#include <stdio.h>

// this value is exposed globally
VALUE klass_message;

static void free_message(void *ptr) {
  frame_t *frame = (frame_t *)ptr;

  frame_free(frame);
}

VALUE alloc_message(frame_t *frame) {
  return rb_data_object_alloc(klass_message, frame, 0, free_message);
}

static VALUE get_command(VALUE self) {
  frame_t *frame;

  Data_Get_Struct(self, frame_t, frame);

  return rb_str_new2(frame->cmd);
}

static VALUE get_headers(VALUE self) {
  struct data_entry *entry;
  frame_t *frame;
  VALUE arr = rb_ary_new();

  Data_Get_Struct(self, frame_t, frame);

  int i = 0;
  list_for_each_entry(entry, &frame->h_headers, list) {
    rb_ary_store(arr, i++, rb_str_new2(entry->data));
  }

  return arr;
}

static VALUE get_body(VALUE self) {
  struct data_entry *entry;
  frame_t *frame;
  VALUE arr = rb_ary_new();

  Data_Get_Struct(self, frame_t, frame);

  int i = 0;
  list_for_each_entry(entry, &frame->h_body, list) {
    rb_ary_store(arr, i++, rb_str_new2(entry->data));
  }

  return rb_funcall(arr, rb_intern("join"), 0);
}

void init_klass_message(VALUE module) {
  klass_message = rb_define_class_under(module, "Message", rb_cObject);

  rb_define_method(klass_message, "command", get_command, 0);
  rb_define_method(klass_message, "headers", get_headers, 0);
  rb_define_method(klass_message, "body", get_body, 0);
}
