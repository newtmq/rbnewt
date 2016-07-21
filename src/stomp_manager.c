#include <stomp/common.h>
#include <stomp/frame.h>
#include <stomp/list.h>

#include <newt/stomp_manager.h>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>

static void init_client_worker(stomp_worker_t *worker) {
  assert(worker != NULL);

  worker->session = stomp_init();
  if(worker->session != NULL) {
    frame_t *frame = NULL;
    int ret;

    ret = stomp_connect(worker->session, worker->host, worker->port, worker->userid, worker->passwd);
    if(ret == RET_ERROR) {
      stomp_cleanup(worker->session);
    }

    do {
      frame = stomp_recv(worker->session);
    } while(frame == NULL);

    SET(worker, STOMP_WORKER_CONNECTED);
  }
}
static void cleanup_client_worker(stomp_worker_t *worker) {
  stomp_cleanup(worker->session);
}

static void *stomp_client_worker(void *arg) {
  stomp_worker_t *worker = (stomp_worker_t *)arg;

  assert(worker != NULL);
  assert(worker->manager != NULL);

  init_client_worker(worker);

  while(! GET(worker, STOMP_WORKER_STATUS_STOP)) {
    frame_t *frame = NULL;

    pthread_mutex_lock(&worker->manager->m_frames);
    if(! list_empty(&worker->manager->h_frames)) {
      frame = list_first_entry(&worker->manager->h_frames, frame_t, list);
      list_del(&frame->list);
    }
    pthread_mutex_unlock(&worker->manager->m_frames);

    if(frame != NULL) {
      frame_send(frame, worker->session->conn);
      frame_free(frame);
    }
  }
  cleanup_client_worker(worker);

  return NULL;
}

static stomp_worker_t *alloc_worker() {
  stomp_worker_t *worker;

  worker = (stomp_worker_t *)malloc(sizeof(stomp_worker_t));
  if(worker != NULL) {
    worker->session = NULL;
    worker->manager = NULL;
    worker->status = 0;
  }

  return worker;
}
static void free_worker(stomp_worker_t *worker) {
  free(worker);
}

static stomp_manager_t *alloc_manager(int concurrency) {
  stomp_manager_t *manager;

  manager = (stomp_manager_t *)malloc(sizeof(stomp_manager_t));
  if(manager != NULL) {
    manager->concurrency = concurrency;

    manager->workers = (stomp_worker_t **)malloc(sizeof(void *) * concurrency);
    if(manager->workers == NULL) {
      free(manager);
      return NULL;
    }

    INIT_LIST_HEAD(&manager->h_frames);
    pthread_mutex_init(&manager->m_frames, NULL);

    int i;
    for(i=0; i<concurrency; i++) {
      manager->workers[i] = alloc_worker();
      manager->workers[i]->manager = manager;
    }
  }

  return manager;
}
static void free_manager(stomp_manager_t *manager) {
  int i;

  if(manager != NULL) {
    for(i=0; i<manager->concurrency; i++) {
      free_worker(manager->workers[i]);
    }

    free(manager->workers);
    free(manager);
  }
}

// The argument 'c' means concurrency
stomp_manager_t *stomp_manager_init(int c) {
  return alloc_manager(c);
}
void stomp_manager_cleanup(stomp_manager_t *manager) {
  if(manager != NULL) {
    int i;
    for(i=0; i<manager->concurrency; i++) {
      free_worker(manager->workers[i]);
    }

    frame_t *frame, *f;
    pthread_mutex_lock(&manager->m_frames);
    {
      list_for_each_entry_safe(frame, f, &manager->h_frames, list) {
        list_del(&frame->list);
        frame_free(frame);
      }
    }
    pthread_mutex_unlock(&manager->m_frames);

    free(manager->workers);
    free(manager);
  }
}

int stomp_worker_start(stomp_manager_t *manager, char *host, int port, char *userid, char *passwd) {
  assert(manager != NULL);

  int i;
  for(i=0; i<manager->concurrency; i++) {
    // set STOMP server information to connect
    manager->workers[i]->host = host;
    manager->workers[i]->port = port;
    manager->workers[i]->userid = userid;
    manager->workers[i]->passwd = passwd;

    pthread_create(&manager->workers[i]->wid, NULL, &stomp_client_worker, manager->workers[i]);

    while(! GET(manager->workers[i], STOMP_WORKER_CONNECTED)) {
      pthread_yield();
    }
  }

  return RET_SUCCESS;
}

int stomp_worker_stop(stomp_manager_t *manager) {
  assert(manager != NULL);

  int i;
  for(i=0; i<manager->concurrency; i++) {
    SET(manager->workers[i], STOMP_WORKER_STATUS_STOP);

    pthread_cancel(manager->workers[i]->wid);
    pthread_join(manager->workers[i]->wid, NULL);
  }

  return RET_SUCCESS;
}

int stomp_manager_send(stomp_manager_t *manager, frame_t *frame) {
  assert(manager != NULL);
  assert(frame != NULL);

  pthread_mutex_lock(&manager->m_frames);
  {
    list_add_tail(&frame->list, &manager->h_frames);
  }
  pthread_mutex_unlock(&manager->m_frames);

  return RET_SUCCESS;
}

frame_t *stomp_manager_recv(stomp_manager_t *manager) {
  frame_t *frame = NULL;

  assert(manager != NULL);

  do {
    int i;
    for(i=0; i<manager->concurrency; i++) {
      frame = stomp_recv(manager->workers[i]->session);
      if(frame != NULL) {
        break;
      }
    }
  } while(frame == NULL);

  return frame;
}
