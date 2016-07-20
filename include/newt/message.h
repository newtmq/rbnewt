#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <ruby/ruby.h>
#include <stomp/frame.h>

extern VALUE klass_message;

VALUE alloc_message(frame_t *);
void init_klass_message(VALUE module);

#endif
