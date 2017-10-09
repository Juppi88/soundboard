#include "main.h"
#include "sounds.h"
#include "utils.h"
#include "httpserver/httpserver.h"
#include <stdio.h>
#include <time.h>

// The folder in which all the sound category subolders are located.
// TODO: Supply this as a command line argument?
static const char *directory = "sounds";

#define JSON_MIME_TYPE "application/json";

#define RESULT_JSON \
"{"\
"	\"result\": %s"\
"}"

// --------------------------------------------------

static struct http_response_t handle_request(struct http_request_t *request)
{
	char result_buffer[32];

	struct http_response_t response;
	memset(&response, 0, sizeof(response));

	response.message = HTTP_200_OK;

	// Get the current time for logging.
	time_t timestamp = time(NULL);
	struct tm *now = localtime(&timestamp);

	// Retrieve and return a list of sounds loaded into the system.
	if (strcmp("/list/", request->request) == 0) {

		printf("[%02d:%02d.%02d] (%s) Sound list was requested.\n", now->tm_hour, now->tm_min, now->tm_sec, request->requester);

		response.content = sounds_get_json_list();
		response.content_type = JSON_MIME_TYPE;
	}

	// Refresh the sound list.
	// This will go through all the sound files so it's best to do it only on request and not every time someone requests the sound list.
	else if (strcmp("/refresh/", request->request) == 0) {

		printf("[%02d:%02d.%02d] (%s) Refreshing sound lists...\n\n", now->tm_hour, now->tm_min, now->tm_sec, request->requester);

		sounds_shutdown();
		sounds_initialize(directory);

		snprintf(result_buffer, sizeof(result_buffer), RESULT_JSON, "true");

		response.content = result_buffer;
		response.content_type = JSON_MIME_TYPE;
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
			printf("[%02d:%02d.%02d] (%s) Playing sound '%s/%s'.\n", now->tm_hour, now->tm_min, now->tm_sec, request->requester, category, s);

			snprintf(result_buffer, sizeof(result_buffer), RESULT_JSON, "true");
		}
		else {
			snprintf(result_buffer, sizeof(result_buffer), RESULT_JSON, "false");
		}

		response.content = result_buffer;
		response.content_type = JSON_MIME_TYPE;
	}

	// Unsupported request!
	else {
		snprintf(result_buffer, sizeof(result_buffer), RESULT_JSON, "false");

		response.message = HTTP_400_BAD_REQUEST;
		response.content = result_buffer;
		response.content_type = JSON_MIME_TYPE;
	}

	return response;
}

int main(void)
{
	// Load the sounds in the sound folder.
	sounds_initialize(directory);

	// Initialize the web interface.
	struct server_ssettings_t settings;
	memset(&settings, 0, sizeof(settings));

	settings.handler = handle_request;
	settings.port = 8000;
	settings.timeout = 10;
	settings.max_connections = 25;
	settings.connection_timeout = 60;

	// Get static web files from the 'web' folder.
	struct server_directory_t directories[] = { { "/", "web" } };

	settings.directories = directories;
	settings.directories_len = 1;

	if (!http_server_initialize(settings)) {

		printf("Failed to start the server!\n");
		return 0;
	}

	printf("Listening for web API requests on port %u.\n\n", settings.port);

	// Loop forever.
	for (;;) {
		http_server_listen();
	}

	// Clean up and free memory.
	sounds_shutdown();
	http_server_shutdown();

	return 0;
}
