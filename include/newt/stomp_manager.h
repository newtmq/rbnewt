#ifndef __STOMP_MANAGER_H__
#define __STOMP_MANAGER_H__

#include <stomp/common.h>
#include <stomp/stomp.h>
#include <stomp/frame.h>
#include <stomp/list.h>

enum stomp_worker_status {
  STOMP_WORKER_CONNECTED = (1 << 0),
  STOMP_WORKER_STATUS_STOP = (1 << 1),
};

typedef struct stomp_manager stomp_manager_t;

// This data structure is closed in stomp_worker
typedef struct stomp_worker {
  pthread_t wid;
  stomp_session_t *session;
  stomp_manager_t *manager;

  int status;

  // informations to connect with STOMP server (Note: these member is impermanent)
  char *host, *userid, *passwd;
  int port;
} stomp_worker_t;

// This is singleton data structure
struct stomp_manager {
  stomp_worker_t **workers;
  int concurrency;

  struct list_head h_frames;
  pthread_mutex_t m_frames;
};

stomp_manager_t *stomp_manager_init(int);
void stomp_manager_cleanup(stomp_manager_t *);

int stomp_worker_start(stomp_manager_t *, char *, int, char *, char *);
int stomp_worker_stop(stomp_manager_t *);

int stomp_manager_send(stomp_manager_t *, frame_t *);
frame_t *stomp_manager_recv(stomp_manager_t *);

#endif
