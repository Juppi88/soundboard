#include "main.h"
#include "sounds.h"
#include "httpserver/httpserver.h"
#include <stdio.h>

// The folder in which all the sound category subolders are located.
// TODO: Supply this as a command line argument?
static const char *directory = "sounds";

// --------------------------------------------------

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

	// Refresh the sound list.
	// This will go through all the sound files so it's best to do it only on request and not every time someone requests the sound list.
	if (strcmp("/refresh/", request->request) == 0) {

		response.message = HTTP_200_OK;

		printf("Refreshing sound lists...\n");

		sounds_shutdown();
		sounds_initialize(directory);
	}

	// Play a sound with the given filename.
	else if (strncmp("/play/", request->request, 6) == 0) {

		char req[128];
		strncpy(req, request->request, sizeof(req));

		// Get the requested category and sound name from the request URL.
		char *category = &req[6], *s = category;

		while (*s) {
			if (*s == '/') {
				*s++ = 0;
				break;
			}
			++s;
		}

		if (*category != 0 && *s != 0) {

			// TODO: Check that the sound actually exists.
			sounds_play(category, s);
			printf("Playing sound '%s/%s'.\n", category, s);
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
	// Load the sounds in the sound folder.
	sounds_initialize(directory);

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
