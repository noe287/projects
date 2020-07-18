#include "common.h"

int update_rentry(char *key, char *new_record) {
	struct route_entry *route = NULL;

	TAILQ_FOREACH(route, &route_list_head, route_next_prev) {
		if (strcmp(key, route->gw_ip) == 0) {
			strcpy(route->dest_ip, new_record);
			printf("Entry Updated...\n");
			break;
		}
	}
	return 0;
}

int delete_rentry(char *key){

	struct route_entry *route = NULL;
	struct route_entry *tmp_route = NULL;

	for (route = TAILQ_FIRST(&route_list_head); route != NULL; route = tmp_route) {
                tmp_route = TAILQ_NEXT(route, route_next_prev);
		if (strcmp(key, route->gw_ip) == 0) {
                        /* Remove the item from the tail queue. */
                        TAILQ_REMOVE(&route_list_head, route, route_next_prev);
                        /* Free the item as we don’t need it anymore. */
                        free(route);
			printf("Entry Deleted...\n");
                        break;
                }
        }
	return 0;
}

void dump_table() {
	struct route_entry *route = NULL;

	printf("\nDestination IP    Gateway IP    Output Iface \n ");
        printf("=============== ============== ============== \n");
	TAILQ_FOREACH(route, &route_list_head, route_next_prev) {
		printf("%12s/%d %14s %10s \n",route->dest_ip,route->mask, route->gw_ip, route->out_iface);
        }
}

int create_rentry(struct route_entry *re) {
	char dest_ip[16];
	char gw_ip[16];
	char out_iface[8];
	int mask;

	printf("Enter dest_ip:");
	fflush(stdout);
	scanf("%s", dest_ip);
	printf("Enter netmask:");
	fflush(stdout);
	scanf("%d", &mask);
	printf("Enter GWIp:");
	fflush(stdout);
	scanf("%s", gw_ip);
	printf("Enter OutpuInterface:");
	fflush(stdout);
	scanf("%s", out_iface);

	strncpy(re->dest_ip, dest_ip, sizeof(dest_ip));
	strncpy(re->gw_ip, gw_ip, sizeof(gw_ip));
	strncpy(re->out_iface, out_iface, sizeof(out_iface));
	re->mask = mask;
	printf("Destination IP    Gateway IP    Output Iface  \n");
	printf("=============== ============== ============== \n");
	printf("%12s/%d %14s %10s \n", re->dest_ip, re->mask, re->gw_ip, re->out_iface);


	return 0;
}


