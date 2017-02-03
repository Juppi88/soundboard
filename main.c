#include "main.h"
#include "sounds.h"
#include "httpserver/httpserver.h"
#include <stdio.h>
//#include <time.h>

static struct http_response_t handle_request(struct http_request_t *request)
{
	struct http_response_t response;
	memset(&response, 0, sizeof(response));
	
	// Retrieve and return a list of sounds loaded into the system.
	if (strcmp("/list/", request->request) == 0) {
		
		response.message = HTTP_200_OK;
		response.content = sounds_get_json_list();
		response.content_type = "text/json";

		printf("Sound list was requested.\n");
	}

	// Play a sound with the given filename.
	else if (strncmp("/play/", request->request, 6) == 0) {

		const char *sound = &request->request[6];

		if (*sound != 0) {

			// TODO: Check that the sound actually exists.
			sounds_play(sound);
			printf("Playing sound '%s'.\n", sound);
		}
	}

	else {
		response.message = HTTP_404_NOT_FOUND;
	}
	
	//printf("HTTP request - method: %s, protocol: %s, request: %s\n", request->method, request->protocol, request->request);

	return response;
}

int main(void)
{
	// Seed RNG.
	//srand((uint32_t)time(NULL));

	const char *directory = "statusd112";

	// Load the sounds in the sound folder.
	sounds_initialize(directory);

	printf("Loaded %u sounds in directory %s.\n", sound_count, directory);

	// Initialize the web interface.
	if (!http_server_initialize(8000, handle_request)) {
		printf("Failed to start the server!\n");
		return 0;
	}

	// Loop forever.
	for (;;) {
		http_server_listen();
		Sleep(100);
	}

	// Clean up and free memory.
	sounds_shutdown();
	http_server_shutdown();

	return 0;
}
