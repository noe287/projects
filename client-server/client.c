#include <errno.h>
#include <common.h>


#define SOCKET_NAME "CommSock"
#define BUFFER_SIZE 128


int main(int argc, char *argv[])
{
	struct sockaddr_un addr;
	int size = 0;
	int ret = 0;
	int data_socket;
	sync_msg_t received_msg;
	struct route_entry *route = NULL;

	TAILQ_INIT(&route_list_head);

	data_socket = socket(AF_UNIX, SOCK_STREAM, 0);

	if (data_socket == -1){
		perror("Socket failure");
		exit(EXIT_FAILURE);
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));

	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path));

	ret = connect(data_socket, (const struct sockaddr *) &addr, sizeof(struct sockaddr_un));
	if (ret == -1){
		fprintf(stderr, "The server is not reachable\n");
		exit(EXIT_FAILURE);
	}

	while(1) {
		if ((size = recv(data_socket, (void *) &received_msg,sizeof(sync_msg_t), 0)) > 0) {
			printf("Msg received with size %d bytes\n", size);
			switch(received_msg.opcode) {
				case CREATE:
					printf("Creating route entry\n");
					printf("r.gw_ip %s\n", received_msg.msg.gw_ip);
					printf("r.dest_ip %s\n", received_msg.msg.dest_ip);
					printf("r.out_iface %s\n", received_msg.msg.out_iface);
					printf("r.mask %d\n", received_msg.msg.mask);
					route = (struct route_entry *)malloc(sizeof(struct route_entry));
					if (route == NULL) {
						printf("Cannot allocate memory for the route entry\n");
					}
					strncpy(route->dest_ip, received_msg.msg.dest_ip, sizeof(received_msg.msg.dest_ip));
					strncpy(route->gw_ip, received_msg.msg.gw_ip, sizeof(received_msg.msg.gw_ip));
					strncpy(route->out_iface, received_msg.msg.out_iface, sizeof(received_msg.msg.out_iface));
					route->mask = received_msg.msg.mask;
					TAILQ_INSERT_TAIL(&route_list_head, route, route_next_prev);
					dump_table();
					break;
				case UPDATE:
					printf("Updating entry with gw_ip %s with new dest_ip %s\n", received_msg.msg.gw_ip, received_msg.msg.dest_ip);
					update_rentry(received_msg.msg.gw_ip,received_msg.msg.dest_ip);
					dump_table();
					break;
				case DELETE:
					printf("Deleting entry with gw_ip %s\n", received_msg.msg.gw_ip);
					delete_rentry(received_msg.msg.gw_ip);
					dump_table();
					break;
				default:
					printf("No such operation is defined, retry\n");
			}
		} else {
			printf("Received msg size does not match the expected one, closing connection\n");
			close(data_socket);
			exit(EXIT_SUCCESS);
		}
	}
}
