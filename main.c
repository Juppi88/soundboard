#include "main.h"
#include "sounds.h"
#include "utils.h"
#include "httpserver/httpserver.h"
#include <stdio.h>

// The folder in which all the sound category subolders are located.
// TODO: Supply this as a command line argument?
static const char *directory = "sounds";
static char file_buffer[100000];

// --------------------------------------------------

static struct http_response_t handle_request(struct http_request_t *request)
{
	printf("HTTP request - method: %s, protocol: %s, host: %s, request: %s\n", request->method, request->protocol, request->hostname, request->request);

	struct http_response_t response;
	memset(&response, 0, sizeof(response));

	response.message = HTTP_200_OK;
	
	// Retrieve and return a list of sounds loaded into the system.
	if (strcmp("/list/", request->request) == 0) {

		printf("Sound list was requested.\n");

		response.content = sounds_get_json_list();
		response.content_type = "text/json";
	}

	// Refresh the sound list.
	// This will go through all the sound files so it's best to do it only on request and not every time someone requests the sound list.
	else if (strcmp("/refresh/", request->request) == 0) {

		printf("\nRefreshing sound lists...\n");

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

	// Serve static files (html, css, js).
	else {
		const char *req = request->request;

		if (strcmp(req, "/") == 0) {
			req = "/index.html";
		}

		char path[MAX_PATH];
		snprintf(path, sizeof(path), "./web%s", req);

		FILE *file = fopen(path, "r");

		// Requested file was not found!
		if (file == NULL) {
			response.message = HTTP_404_NOT_FOUND;
			return response;
		}

		// Set the correct MIME type.
		if (string_ends_with(req, ".html")) {
			response.content_type = "text/html";
		}
		else if (string_ends_with(req, ".css")) {
			response.content_type = "text/css";
		}
		else if (string_ends_with(req, ".js")) {
			response.content_type = "application/javascript";
		}
		else {
			response.content_type = "text/plain";
		}

		// Get the size of the file.
		fseek(file, 0, SEEK_END);
		long length = ftell(file);
		fseek(file, 0, SEEK_SET);

		// Read the contents of the file into a buffer which we can send to the requester.
		if (length >= sizeof(file_buffer)) {
			length = sizeof(file_buffer) - 1;
		}

		size_t read = fread(file_buffer, 1, length, file);
		fclose(file);

		file_buffer[read] = 0;
		response.content = file_buffer;
	}

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
		Sleep(10);
	}

	// Clean up and free memory.
	sounds_shutdown();
	http_server_shutdown();

	return 0;
}
