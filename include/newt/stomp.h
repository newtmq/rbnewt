#ifndef __NEWT_STOMP_H__
#define __NEWT_STOMP_H__

#include <newt/common.h>
#include <stomp/stomp.h>

#define NEWT_STOMP_KEY_SERVER "server"
#define NEWT_STOMP_KEY_USERID "userid"
#define NEWT_STOMP_KEY_PASSWD "passwd"
#define NEWT_STOMP_KEY_PORT "port"

#define NEWT_STOMP_DEFAULT_SERVER "localhost"
#define NEWT_STOMP_DEFAULT_USERID "guest"
#define NEWT_STOMP_DEFAULT_PASSWD "guest"
#define NEWT_STOMP_DEFAULT_PORT "61613"

#define NEWT_STOMP_CB_CONNECTED "cb_connected"
#define NEWT_STOMP_CB_MESSAGE "cb_message"
#define NEWT_STOMP_CB_ERROR "cb_error"

int cnewt_stomp_initialize(stomp_session_t *, char *, char *, char *, char *);

#endif
