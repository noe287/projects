#include <stdio.h>
#include <stdint.h>
#include <sys/queue.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

TAILQ_HEAD(, route_entry) route_list_head;
//TAILQ_HEAD(route_list_head, route_entry);
struct route_entry {
	TAILQ_ENTRY(route_entry) route_next_prev;
	char dest_ip[16];
	int mask;
	char gw_ip[16];
	char out_iface[8];
};


enum opcode {
	CREATE = 0,
	UPDATE,
	DELETE,
	DUMP,
	QUIT
};

typedef struct msg_body {
	char dest_ip[16];
	int mask;
	char gw_ip[16];
	char out_iface[8];
} msg_body_t;

typedef struct sync_msg {
	int opcode;
	msg_body_t msg;
} sync_msg_t;

int update_rentry(char *key, char *new_record);
int delete_rentry(char *rentry);
void dump_table();
int create_rentry(struct route_entry *re);

