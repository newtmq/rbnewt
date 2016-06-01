#include <stomp/stomp.h>
#include <newt/common.h>
#include <newt/stomp.h>
#include <pthread.h>

static void *thread_stomp(void *data) {
  stomp_session_t *session = (stomp_session_t *)data;

  //here is main-loop
  stomp_run(session);

  stomp_session_free(session);
}

int cnewt_stomp_initialize(stomp_session_t *session, char *server, char *port, char *userid, char *passwd) {
  const struct stomp_hdr connection_headers[] = {
    {"login",    userid},
    {"passcode", passwd},
  };
  int ret = RET_ERROR;
  pthread_t *thread_id = (pthread_t *)session->ctx;

  if(thread_id != NULL) {
    stomp_connect(session, server, port, 2, connection_headers);

    pthread_create(thread_id, NULL, thread_stomp, session);

    ret = RET_SUCCESS;
  }

  return ret;
}

int cnewt_stomp_close(stomp_session_t *session) {
  pthread_t *thread_id = (pthread_t *)session->ctx;
  const struct stomp_hdr disconnect_header[] = {{0}};
  int ret = RET_ERROR;

  if(thread_id != NULL) {
    // clear run flag to escape stomp mainloop
    session->run = 0;

    pthread_join(*thread_id, NULL);

    ret = RET_SUCCESS;
  }

  return ret;
}
