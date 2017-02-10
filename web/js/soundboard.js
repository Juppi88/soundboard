var soundboardServer = "http://localhost:8000";
var folders = null;
var currentFolder = "statusd112";
var sounds = null;
var filtered = null;
var filterText = "";

function soundboard_initialize()
{
	soundboardServer = document.location.origin;

	// Load the previously used folder from local storage.
	var previousFolder = localStorage.getItem("folder");

	if (previousFolder != null) {
		currentFolder = previousFolder;
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
		if (filterText.length == 0 || sounds[i].toLowerCase().indexOf(filterText) > -1) {
			filtered.push(sounds[i]);
		}
	}

	// Create the buttons for the filtered sounds.
	soundboard_populate_sound_list();
	soundboard_update_folders();
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
	return '<div class="sound" onclick="soundboard_play_sound(\'' + sound + '\');">' + sound + '</div>';
}

function soundboard_play_sound(sound)
{
	var req = new XMLHttpRequest();
		
	req.open("GET", soundboardServer + "/play/" + currentFolder + "/" + sound, true);
	req.send();
}

function soundboard_on_filter_change()
{
	var filterField = document.getElementById("filter");
	if (filterField == null) {
		return;
	}

	filterText = filterField.value.toLowerCase();
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
}

function soundboard_play_first_sound()
{
	var filterField = document.getElementById("filter");
	if (filterField == null) {
		return;
	}

	filterText = filterField.value.toLowerCase();
	soundboard_filter();
}

function soundboard_play_first_sound()
{
	if (filtered.length != 0) {
		soundboard_play_sound(filtered[0]);
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
