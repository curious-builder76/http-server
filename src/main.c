#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "HTTP_Server.h"
#include "hash_table.h"
#include "Response.h"



int main(int argc, char *argv[]) {
	// initiate HTTP_Server
	HTTP_Server http_server;
	int port = 8000;
	if( argc == 2 ) {
		port = atoi(argv[1]);
		if (port == 0) {
			printf("Port %s isn't valid\n", argv[1]);
			exit(-1);
		}
	}
	init_server(&http_server, port);

	int client_socket;

	// registering Routes
	ht * routes=ht_create();
       	ht_set(routes,"/", "index.html"); 
	ht_set(routes, "/about", "about.html");
	
	response_init();
	
	// display all available routes
	printf("\n====================================\n");
	printf("=========ALL VAILABLE ROUTES========\n");
	hti it = ht_iterator(routes);
	while(ht_next(&it))
	{
		printf("KEY: %s, VALUE: %s\n", it.key, (char*)it.value);
	}

	while (1) {
		char client_msg[4096] = "";

		client_socket = accept(http_server.socket, NULL, NULL);

		read(client_socket, client_msg, 4095);
		printf("%s\n", client_msg);

		// parsing client socket header to get HTTP method, route
		char *method = "";
		char *urlRoute = "";

		char *client_http_header = strtok(client_msg, "\n");

		printf("\n\n%s\n\n", client_http_header);

		char *header_token = strtok(client_http_header, " ");

		int header_parse_counter = 0;

		while (header_token != NULL) {

			switch (header_parse_counter) {
				case 0:
					method = header_token;
					/* fall through */
				case 1:
					urlRoute = header_token;
			}
			header_token = strtok(NULL, " ");
			header_parse_counter++;
		}

		printf("The method is %s\n", method);
		printf("The route is %s\n", urlRoute);


		char template[100] = "";
		int status_code=200;
		if (memcmp("/static/",urlRoute,strlen("/static/"))==0){
			(!strcmp("/static/",urlRoute)) ? strcat(template,"index.css") : strcat(template,urlRoute);
			printf("yup!\n");
			fflush(stdout);
		}else {
			char* destination = ht_get(routes, urlRoute);
			strcat(template, "templates/");

			if (destination == NULL) {
				strcat(template, "404.html");
				status_code=404;
			}else {
				strcat(template, destination);
			}
		}

		xsendfile(client_socket,template,status_code);
		close(client_socket);
	}
	response_quit();
	return 0;
}
