#ifndef __NEWT_RBNEWT_H__
#define __NEWT_RBNEWT_H__

#include <stomp/stomp.h>
#include <ruby/ruby.h>

/* internal data structure to share in the session */
typedef struct rbnewt_context_t {
  VALUE callback_obj;
} rbnewt_context_t;

void newt_stomp_callback_connected(stomp_session_t *, void *, void *);
void newt_stomp_callback_message(stomp_session_t *, void *, void *);
void newt_stomp_callback_error(stomp_session_t *, void *, void *);

#endif
