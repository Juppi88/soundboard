var soundboardServer = "http://localhost:8000";
var folders = null;
var currentFolder = "statusd112";
var sounds = null;
var filtered = null;
var filterList = [];
var filterContains = true;

var touchTimer = null;
var touchEnded = 0;
const TouchTime = 1000;

// Queue variables
var soundQueue = [];
var currentQueueSound = 0;
var soundQueueTimer = null;

// --------------------------------------------------------------------------------

function soundboard_initialize()
{
	soundboardServer = document.location.origin;

	// Load the previously used folder from local storage.
	var previousFolder = localStorage.getItem("folder");

	if (previousFolder != null) {
		currentFolder = previousFolder;
	}

	// Load the previously used filter mode.
	var previousMode = localStorage.getItem("filtermode");

	if (previousMode != null) {
		filterContains = parseInt(previousMode);
	}

	// Register enter press to play the first sound in the filtered sound list.
	var filterField = document.getElementById("filter");

	if (filterField != null) {

		filterField.onkeypress = function(e) {
			if (!e) {
				e = window.event;
			}

			var keyCode = e.keyCode || e.which;
			if (keyCode == '13') {
				soundboard_play_first_sound();
			}
		}
	}

	// Register an event listener to cancel tap events on mobile devices.
	document.ontouchend = function(e) {

			if (touchTimer != null) {

				clearTimeout(touchTimer);

				touchTimer = null;
				touchEnded = new Date();
			}
		};

	// Clear and hide the sound queue until new entries are added to it.
	soundboard_populate_queue();

	// Get the list of sounds from the web api server and create the buttons for each sound.
	soundboard_get_sound_list();
}

function soundboard_get_sound_list()
{
	var req = new XMLHttpRequest();

	req.onreadystatechange = function() {

		if (this.readyState == 4 && this.status == 200) {
			var data = JSON.parse(req.responseText);

			if (data != null) {

				// If the received JSON is valid, create the buttons for the sounds.
				folders = data.folders;
				soundboard_on_filter_change();
			}
		}
	}

	req.open("GET", soundboardServer + "/list/");
	req.send();
}

function soundboard_filter()
{
	// Clear the old filtered list.
	filtered = [];
	sounds = null;

	if (folders == null) {
		return;
	}

	// Get the used folder.
	for (var i = 0, len = folders.length; i < len; ++i) {

		if (folders[i].name == currentFolder) {
			sounds = folders[i].sounds;
			break;
		}
	}

	if (sounds == null) {
		return;
	}

	// Go through the sound list and filter the wanted sounds.
	for (var i = 0, len = sounds.length; i < len; ++i) {
		if (soundboard_sound_matches_filter_list(sounds[i].name)) {
			filtered.push(sounds[i]);
		}
	}

	// Sort the sound list based on the order of filter strings.
	filtered.sort(soundboard_compare_sounds);

	// Create the buttons for the filtered sounds.
	soundboard_populate_sound_list();
	soundboard_update_folders();
}

function soundboard_compare_sounds(x, y)
{
	x = x.name.toLowerCase();
	y = y.name.toLowerCase();

	// Compare which sound is first in the filter list.
	if (filterList.length > 1)
	{
		var xIndex = 1000, yIndex = 1000;

		for (var i = 0, c = filterList.length; i < c; ++i) {

			if (soundboard_sound_matches_filter_string(x, filterList[i])) {
				xIndex = i;
				break;
			}
		}

		for (var i = 0, c = filterList.length; i < c; ++i) {
			if (soundboard_sound_matches_filter_string(y, filterList[i])) {
				yIndex = i;
				break;
			}
		}

		if (xIndex < yIndex) return -1;
		if (xIndex > yIndex) return 1;
	}

	// If they're matched by the same filter, compare sound names.
	if (x < y) return -1;
	if (x > y) return 1;

	return 0;
}

function soundboard_sound_matches_filter_list(soundName)
{
	// No filters, accept any sound.
	if (filterList.length == 0) {
		return true;
	}

	// Convert sound name to lower case for comparison.
	soundName = soundName.toLowerCase();

	for (var i = 0, len = filterList.length; i < len; ++i) {

		if (soundboard_sound_matches_filter_string(soundName, filterList[i])) {
			return true;
		}
	}

	// No matching filter was found, hide the sound.
	return false;
}

function soundboard_sound_matches_filter_string(soundName, filter)
{
	// Filter string starts and ends with a quote, the sound name must match the entire filter.
	if (filter[0] == '"' &&
		filter[filter.length - 1] == '"') {
		return (soundName == filter.replace(/"/g,""));
	}

	else {
		var subStringIdx = soundName.indexOf(filter);

		if ((filterContains && subStringIdx > -1) ||	// Filter mode: contains filter text
			(!filterContains && subStringIdx == 0)) {	// Filter mode: starts with filter text
			return true;
		}
	}

	return false;
}

function soundboard_populate_sound_list()
{
	// Get the container div for the sound list.
	var soundContainer = document.getElementById("sounds");
	if (soundContainer == null) {
		return;
	}

	// Format the contents of the sound list as HTMl
	html = "";

	for (var i = 0, len = filtered.length; i < len; ++i) {
		html += soundboard_add_button_text(filtered[i]);
	}

	// Set the sound list HTML.
	soundContainer.innerHTML = html;
}

function soundboard_add_button_text(sound)
{
	return '<div class="sound" ontouchstart="soundboard_tap_sound(\'' + sound.name + '\', ' + sound.duration + ');"' +
			' onclick="soundboard_play_sound(event, \'' + sound.name + '\', ' + sound.duration + ');">' + sound.name + '</div>';
}

function soundboard_tap_sound(sound, duration)
{
	if (duration != 0) {
		touchTimer = setTimeout(soundboard_on_sound_tapped, TouchTime, sound, duration);
	}
}

function soundboard_on_sound_tapped(sound, duration)
{
	touchTimer = null;
	soundboard_add_to_queue(sound, duration);
}

function soundboard_play_sound(ev, sound, duration)
{
	// Cancel tap detection timer.
	if (touchTimer != null) {
		clearTimeout(touchTimer);
		touchTimer = null;
	}

	// Don't play a sound right after tapping.
	var now = new Date();
	if (touchEnded != null && now - touchEnded < 500) {
		//return;
	}

	// If the control key is pressed while clicking, add the sound to the queue.
	if (duration != 0 && ev != null && ev.ctrlKey) {
		soundboard_add_to_queue(sound, duration);
		return;
	}

	// Otherwise play the sound immediately.
	var req = new XMLHttpRequest();
		
	req.open("GET", soundboardServer + "/play/" + currentFolder + "/" + sound, true);
	req.send();
}

function soundboard_add_to_queue(sound, duration)
{
	soundQueue.push([ sound, duration ]);
	soundboard_populate_queue();
}

function soundboard_populate_queue()
{
	// Get the container divs for the sound queue.
	var emptySpace = document.getElementById("emptyspace");
	var queueContainer = document.getElementById("queuecontainer");
	var queue = document.getElementById("queue");

	if (emptySpace == null || queueContainer == null || queue == null) {
		return;
	}

	// If the queue is empty, hide the queue.
	if (soundQueue.length == 0) {
		queueContainer.style.display = "none";
		emptySpace.style.minHeight = "0";
		return;
	}

	// Make the queue visible.
	queueContainer.style.display = "block";
	emptySpace.style.minHeight = (queueContainer.offsetHeight + 10) + "px"; // Horrible hack to move the sound list a bit down so it isn't blocked by the queue.
	//alert(queueContainer.offsetHeight + "px");

	// Add the queued sounds to the lists as divs.
	var html = "";

	for (var i = 0, len = soundQueue.length; i < len; ++i) {
		html += '<div class="queuedsound">' + soundQueue[i][0] + '</div>';
	}

	// Update the HTML of the queue container.
	queue.innerHTML = html;
}

function soundboard_play_next_in_queue()
{
	// Make sure something hasn't gone wrong and the queue hasn't been cleared while playing it.
	if (currentQueueSound >= soundQueue.length) {
		soundboard_abort_queue()
		return;
	}

	var sound = soundQueue[currentQueueSound][0];
	var duration = soundQueue[currentQueueSound][1];
	
	if (++currentQueueSound >= soundQueue.length) {
		// The queue has finished!
		soundboard_abort_queue();
	}
	else {
		// Set a timer for playing the next sound after the current one has finished.
		soundQueueTimer = setTimeout(soundboard_play_next_in_queue, duration);
	}

	// Send a request to play the sound.
	var req = new XMLHttpRequest();
		
	req.open("GET", soundboardServer + "/play/" + currentFolder + "/" + sound, true);
	req.send();
}

function soundboard_abort_queue()
{
	if (soundQueueTimer != null) {
		clearTimeout(soundQueueTimer);
	}

	soundQueueTimer = null;
	currentQueueSound = 0;
}

function soundboard_on_press_play_queue()
{
	soundboard_abort_queue();
	soundboard_play_next_in_queue();
}

function soundboard_on_press_clear_queue()
{
	soundQueue = [];
	soundboard_populate_queue();

	// If the queue was already being played, abort it.
	soundboard_abort_queue();
}

function soundboard_on_filter_change()
{
	var filterField = document.getElementById("filter");
	if (filterField == null) {
		return;
	}

	var filters = filterField.value;

	// All sound names are converted to lower case, hence why the filters should be as well.
	filters = filters.toLowerCase();

	// Split the filter string into separate filter words.
	filterList = filters.split(" ");

	// Remove empty filters.
	filterList = filterList.filter(function(x) { return x.length != 0 });

	// Update the sound list using the new filters.
	soundboard_filter();
}

function soundboard_on_folder_change()
{
	var folderField = document.getElementById("folder");
	if (folderField == null) {
		return;
	}

	currentFolder = folderField.value.toLowerCase();

	// Store the current folder to local storage so the page remembers it on a later visit.
	localStorage.setItem("folder", currentFolder);

	soundboard_filter();
	soundboard_on_press_clear_queue();
}

function soundboard_on_filter_mode_change()
{
	var modeField = document.getElementById("filtermode");
	if (modeField == null) {
		return;
	}

	filterContains = (modeField.value == "Contains");

	// Store the current mode to local storage so the page remembers it on a later visit.
	localStorage.setItem("filtermode", (filterContains ? 1 : 0).toString());

	soundboard_filter();
}

function soundboard_play_first_sound()
{
	if (filtered.length != 0) {
		soundboard_play_sound(null, filtered[0], 0);
	}
}

function soundboard_compare_folders(x, y)
{
	if (x.name < y.name) return -1;
	if (x.name > y.name) return 1;
	return 0;
}

function soundboard_update_folders()
{
	if (folders == null) {
		return;
	}

	folders.sort(soundboard_compare_folders);

	// Get the container div for the folder list.
	var folderContainer = document.getElementById("folder");
	if (folderContainer == null) {
		return;
	}

	// Format the contents of the sound list as HTMl
	html = "";

	for (var i = 0, len = folders.length; i < len; ++i) {
		var f = folders[i];
		html += "<option value=\"" + f.name + "\"" + (f.name == currentFolder ? " selected" : "") + ">" + f.name + " (" + f.sounds.length + ")</option>"
	}

	// Set the sound list HTML.
	folderContainer.innerHTML = html;
}

function soundboard_on_press_refresh()
{
	// Send a refresh request to the server.
	var req = new XMLHttpRequest();
		
	req.open("GET", soundboardServer + "/refresh/", true);
	req.send();

	// Then request the updated list.
	soundboard_get_sound_list();
}
