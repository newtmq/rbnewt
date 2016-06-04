#include <stomp/stomp.h>

#include <newt/rbnewt.h>
#include <newt/common.h>
#include <newt/stomp.h>

void cnewt_stomp_start(stomp_session_t *session) {
  //here is main-loop
  stomp_run(session);
}

int cnewt_stomp_initialize(stomp_session_t *session, char *server, char *port, char *userid, char *passwd) {
  const struct stomp_hdr connection_headers[] = {
    {"login",    userid},
    {"passcode", passwd},
  };
  int ret = RET_ERROR;
  rbnewt_context_t *context = (rbnewt_context_t *)session->ctx;

  if(context != NULL) {
    stomp_connect(session, server, port, 2, connection_headers);

    stomp_callback_set(session, SCB_CONNECTED, newt_stomp_callback_connected);
    stomp_callback_set(session, SCB_MESSAGE, newt_stomp_callback_message);
    stomp_callback_set(session, SCB_ERROR, newt_stomp_callback_error);

    ret = RET_SUCCESS;
  }

  return ret;
}
