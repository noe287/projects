#include "common.h"
#include <pthread.h>


//TAILQ_HEAD(, route_entry) route_list_head;


#define SOCKET_NAME "CommSock"
#define BUFFER_SIZE 128
#define MAX_CLIENT_SUPPORTED 32

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


int monitored_fd_set[MAX_CLIENT_SUPPORTED];
int ret;
sync_msg_t update_clients;

static void init_monitored_fd_set() {

	int i = 0;
	for(; i < MAX_CLIENT_SUPPORTED; i++)
		monitored_fd_set[i] = -1;
}

static void add_to_monitored_fd_set(int new_fd) {

	int i = 0;
	for(; i < MAX_CLIENT_SUPPORTED; i++) {
		if (monitored_fd_set[i] != -1)
			continue;
		monitored_fd_set[i] = new_fd;
		break;
	}
}

static void rm_from_monitored_fd_set(int rm_fd) {

	int i = 0;

	for (; i < MAX_CLIENT_SUPPORTED; i++) {
		if (monitored_fd_set[i] != rm_fd)
			continue;

		monitored_fd_set[i] = -1;
		break;
	}
}

static void refresh_fd_set(fd_set *fd_set_ptr) {

	FD_ZERO(fd_set_ptr);
	int i = 0;
	for(;i < MAX_CLIENT_SUPPORTED; i++){
		if (monitored_fd_set[i] != -1) {
			FD_SET(monitored_fd_set[i], fd_set_ptr);
		}
	}
}

static int get_max_fd() {
	int i = 0;
	int max = -1;

	for(; i < MAX_CLIENT_SUPPORTED; i++) {
		if (monitored_fd_set[i] > max)
			max = monitored_fd_set[i];
	}
	return max;
}

static void *get_user_input(void *arg)
{
	char gw_ip[16] = {0};
	char dest_ip[16] = {0};
	int opcode;
	struct route_entry *route = NULL;

	for(;;) {
		printf("Please enter type of the operation: Create(0), Update(1), Delete(2), Dump(3), Exit(4)\n");
		fflush(stdout);
		scanf("%d", &opcode);
		switch(opcode) {
			case CREATE:
				printf("Creating route entry\n");
				route = (struct route_entry *)malloc(sizeof(struct route_entry));
				if (route == NULL) {
					printf("Cannot allocate memory for the route entry\n");
					pthread_exit(&ret);
				}
				create_rentry(route);
				TAILQ_INSERT_TAIL(&route_list_head, route, route_next_prev);

				strcpy(update_clients.msg.gw_ip, route->gw_ip);
				strcpy(update_clients.msg.dest_ip, route->dest_ip);
				strcpy(update_clients.msg.out_iface, route->out_iface);
				update_clients.msg.mask = route->mask;

				pthread_mutex_lock(&mtx);
				update_clients.opcode = CREATE;
				pthread_mutex_unlock(&mtx);
				pthread_cond_signal(&cond);
				break;
			case UPDATE:
				printf("Enter Gateway IP:\n");
				fflush(stdout);
				scanf("%s", gw_ip);
				printf("Enter New Destination IP:\n");
				fflush(stdout);
				scanf("%s", dest_ip);

				update_rentry(gw_ip, dest_ip);

				strcpy(update_clients.msg.gw_ip, gw_ip);
				strcpy(update_clients.msg.dest_ip, dest_ip);

				pthread_mutex_lock(&mtx);
				update_clients.opcode = UPDATE;
				pthread_mutex_unlock(&mtx);
				pthread_cond_signal(&cond);
				break;
			case DELETE:
				printf("Enter Gateway IP delete :\n");
				fflush(stdout);
				scanf("%s", gw_ip);

				delete_rentry(gw_ip);

				strcpy(update_clients.msg.gw_ip, gw_ip);

				pthread_mutex_lock(&mtx);
				update_clients.opcode = DELETE;
				pthread_mutex_unlock(&mtx);
				pthread_cond_signal(&cond);
				break;
			case DUMP:
				dump_table();
				break;
			case QUIT:
				printf("Quitting...\n");
				pthread_exit(&ret);
			default:
				printf("No such operation is defined, retry\n");
		}
	}

}

static void *inform_clients(void *arg) {
	sync_msg_t send_msg;
	int nbytes = 0;
	int s = 0;

	for(;;) {
		s = pthread_mutex_lock(&mtx);
		if (s != 0) {
			perror("Cannot lock mutex");
			pthread_exit(&ret);
		}

		while(update_clients.opcode < 0){
			s = pthread_cond_wait(&cond, &mtx);
			if (s != 0) {
				perror("Cannot wait on a cond variable.");
				pthread_exit(&ret);
			}
		}

		while(update_clients.opcode >= 0) {
			/* Now test if our thread signaled us something new */
			int i = 2;
			for(; i < MAX_CLIENT_SUPPORTED; i++) {
				if (monitored_fd_set[i] != -1) {
					if ((nbytes = write(monitored_fd_set[i], &update_clients, sizeof(update_clients)) != sizeof(sync_msg_t)))
					{
						printf("Error informing peers on %d event\n", update_clients.opcode);
					}
				}
			}
			update_clients.opcode = -1;
			/* if (update_clients.opcode == 0) { */
			/* 	for(; i < MAX_CLIENT_SUPPORTED; i++) { */
			/* 		if (monitored_fd_set[i] != -1) { */
			/* 			if ((nbytes = write(monitored_fd_set[i], &update_clients, sizeof(update_clients)) != sizeof(sync_msg_t))) */
			/* 			{ */
			/* 				printf("Error sending new route entry to the clients\n"); */
			/* 			} */
			/* 		} */
			/* 	} */
			/* 	update_clients.opcode = -1; */
			/* } else if (update_clients.opcode == 1) { */
			/* 	int i = 2; */
			/* 	for(; i < MAX_CLIENT_SUPPORTED; i++) { */
			/* 		if (monitored_fd_set[i] != -1) { */
			/* 			if ((nbytes = write(monitored_fd_set[i], &update_clients, sizeof(update_clients)) != sizeof(sync_msg_t))) */
			/* 			{ */
			/* 				printf("Error Sending UPDATE to the clients\n"); */
			/* 			} */
			/* 		} */
			/* 	} */
			/* 	update_clients.opcode = -1; */
			/* } else if (update_clients.opcode == 2) { */
			/* 	int i = 2; */
			/* 	for(; i < MAX_CLIENT_SUPPORTED; i++) { */
			/* 		if (monitored_fd_set[i] != -1) { */
			/* 			if ((nbytes = write(monitored_fd_set[i], &update_clients, sizeof(update_clients)) != sizeof(sync_msg_t))) */
			/* 			{ */
			/* 				  printf("Error Sending DELETE to the clients\n"); */
			/* 			} */
			/* 		} */
			/* 	} */
			/* 	update_clients.opcode = -1; */
			//}
		}
		s = pthread_mutex_unlock(&mtx);
		if (s != 0) {
			perror("Cannot unlock mutex.");
			pthread_exit(&ret);
		}
	}
}

int main() {
	pthread_t input_thread;
	pthread_t inform_thread;
	struct sockaddr_un name;
	struct route_entry *re;
	sync_msg_t send_msg;
	int nbytes = 0;
	int s = 1;
	int master_socket, i = 0;
	int data_socket;
	int comm_socket;
	int data;
	char buffer[BUFFER_SIZE];
	fd_set readfds;

	/*init*/
	init_monitored_fd_set();
	add_to_monitored_fd_set(0);

	TAILQ_INIT(&route_list_head);

	unlink(SOCKET_NAME);

	memset(&update_clients, 0, sizeof(sync_msg_t));

	/*SOCK_DGRAM for datagram based communication*/
	master_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (master_socket == -1){
		perror("Socket");
		exit(EXIT_FAILURE);
	}

	printf("Master Socket Created\n");
	memset(&name, 0, sizeof(struct sockaddr_un));
	name.sun_family = AF_UNIX;
	strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) -1);

	ret = bind(master_socket, (const struct sockaddr *) &name, sizeof(struct sockaddr_un));
	if (ret == -1) {
		perror("Bind error");
		exit(EXIT_FAILURE);
	}
	printf("Success on Bind() call\n");

	ret = listen(master_socket, 20);
	if (ret == -1) {
		perror("Listen error");
		exit(EXIT_FAILURE);
	}
	add_to_monitored_fd_set(master_socket);

	printf("Creating threads\n");
	s = pthread_create(&input_thread, NULL, get_user_input, NULL);
	if (s != 0)
		exit(EXIT_FAILURE);
	s = pthread_create(&inform_thread, NULL, inform_clients, NULL);
	if (s != 0)
		exit(EXIT_FAILURE);


	for(;;) {
		refresh_fd_set(&readfds);
		printf("Waiting on select()\n");
		select(get_max_fd() + 1, &readfds, NULL, NULL, NULL);

		/* Test if there is any new connection */
		if (FD_ISSET(master_socket, &readfds)) {
			data_socket = accept(master_socket, NULL, NULL);
			if (data_socket == -1) {
				perror("Accept error");
				exit(EXIT_FAILURE);
			}
			printf("A new client connection is established \n");
			add_to_monitored_fd_set(data_socket);
			//dump_table();
			if (!TAILQ_EMPTY(&route_list_head)) {
				TAILQ_FOREACH(re, &route_list_head, route_next_prev) {
					strcpy(send_msg.msg.gw_ip, re->gw_ip);
					strcpy(send_msg.msg.dest_ip, re->dest_ip);
					strcpy(send_msg.msg.out_iface, re->out_iface);
					send_msg.msg.mask = re->mask;
					send_msg.opcode = CREATE;
					if ((nbytes = write(data_socket, &send_msg, sizeof(send_msg)) != sizeof(sync_msg_t)))
					{
						  printf("Error Sending tailq route entry to the client\n");
					}
				}
			}

		}

		/* else { */
		/* 	i = 0; comm_socket = -1; */
		/* 	for(; i < MAX_CLIENT_SUPPORTED; i ++) { */
		/* 		if (FD_ISSET(monitored_fd_set[i], &readfds)) { */
		/* 			comm_socket = monitored_fd_set[i]; */
		/* 			memset(buffer, 0, BUFFER_SIZE); */
                /*  */
		/* 			printf("Waiting for data from the client\n"); */
		/* 			ret = read(comm_socket, buffer, BUFFER_SIZE); */
		/* 			if (ret == -1) { */
		/* 				perror("Read error"); */
		/* 				exit(EXIT_FAILURE); */
		/* 			} */
		/* 			memcpy(&data, buffer, sizeof(int)); */
		/* 			if (data == 0) { */
		/* 				memset(buffer, 0, BUFFER_SIZE); */
		/* 				sprintf(buffer, "Result = %d", 10); */
		/* 				printf("Sending result back to the client\n"); */
		/* 				ret = write(comm_socket, buffer, BUFFER_SIZE); */
		/* 				if (ret == -1) { */
		/* 					perror("Write error"); */
		/* 					exit(EXIT_FAILURE); */
		/* 				} */
		/* 				close(comm_socket); */
		/* 				rm_from_monitored_fd_set(comm_socket); */
		/* 				continue; */
		/* 			} */
		/* 		} */
		/* 	} */
		/* } */
	}
	return 0;
}













