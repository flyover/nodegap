/**
 * Copyright (c) Flyover Games, LLC
 */

var boot = function ()
{
	document.body.style.width = '100%';
	document.body.style.height = '100%';
	document.body.style.border = '0px';
	document.body.style.margin = '0px';
	document.body.style.padding = '0px';
	document.body.style['box-sizing'] = 'border-box';
	document.body.style['-moz-box-sizing'] = 'border-box';
	document.body.style['-webkit-box-sizing'] = 'border-box';
	document.body.style.backgroundColor = '#46483e';

	var table_div = document.body.appendChild(document.createElement('div'));
	table_div.style.position = 'absolute';
	table_div.style.display = 'table';
	table_div.style.width = '100%';
	table_div.style.height = '100%';

	var default_source = 
	[
		"console.log(\"Hello, World!\");",
		""
	];

	//var div = table_div.appendChild(document.createElement('div'));
	//var source_textarea = div.appendChild(document.createElement('textarea'));
	//source_textarea.style.width = '100%';
	//source_textarea.rows = 12;
	//source_textarea.style.backgroundColor = 'rgba(255,255,255,0.75)';
	//source_textarea.innerHTML = default_source.join('\n');

	var table_row_div = table_div.appendChild(document.createElement('div'));
	table_row_div.style.display = 'table-row';
	table_row_div.style.width = '100%';
	//table_row_div.style.height = '100%';
	table_row_div.style.background = "url(img/logo.png) no-repeat center center";
	var source_editor_div = table_row_div.appendChild(document.createElement('div'));
	source_editor_div.id = 'source_editor';
	source_editor_div.style.width = '100%';
	source_editor_div.style.height = '100%';
	source_editor_div.style.backgroundColor = 'rgba(255,255,255,0.65)';
	// create ace source editor
	var source_editor = ace.edit('source_editor');
	source_editor.setValue("");
	source_editor.insert(default_source.join('\n'));
	source_editor.renderer.setShowGutter(false);
	source_editor.renderer.setShowPrintMargin(false);
	// set JavaScript mode
	var JavaScriptMode = require("ace/mode/javascript").Mode;
	source_editor.getSession().setMode(new JavaScriptMode());

	var table_row_div = table_div.appendChild(document.createElement('div'));
	table_row_div.style.display = 'table-row';
	table_row_div.style.width = '100%';
	table_row_div.style.height = '0';
	table_row_div.style.textAlign = 'center';
	var exec_button = table_row_div.appendChild(document.createElement('button'));
	exec_button.innerHTML = "RUN";
	exec_button.style.margin = '8px';
	exec_button.style.padding = '8px';
	var clear_button = table_row_div.appendChild(document.createElement('button'));
	clear_button.innerHTML = "CLEAR";
	clear_button.style.margin = '8px';
	clear_button.style.padding = '8px';

	//var div = table_div.appendChild(document.createElement('div'));
	//var output_textarea = div.appendChild(document.createElement('textarea'));
	//output_textarea.style.width = '100%';
	//output_textarea.rows = 8;
	//output_textarea.style.backgroundColor = 'rgba(255,255,255,0.75)';
	//output_textarea.innerHTML = "";

	var table_row_div = table_div.appendChild(document.createElement('div'));
	table_row_div.style.display = 'table-row';
	table_row_div.style.width = '100%';
	table_row_div.style.height = '25%';
	var output_editor_div = table_row_div.appendChild(document.createElement('div'));
	output_editor_div.id = 'output_editor';
	output_editor_div.style.width = '100%';
	output_editor_div.style.height = '100%';
	output_editor_div.style.backgroundColor = 'rgba(255,255,255,0.65)';
	// create ace output editor
	var output_editor = ace.edit('output_editor');
	output_editor.setValue("");
	output_editor.setReadOnly(true);
	output_editor.setHighlightActiveLine(false);
	output_editor.renderer.setShowGutter(false);
	output_editor.renderer.setShowPrintMargin(false);

	source_editor.on('input', function(e){
		
		// disable by default
		// this will be changed in the if statement below
		exec_button.disabled = true;

		var code = source_editor.getValue();

		// check if there is no code to be executed
		if (code.length > 0) {
			exec_button.disabled = false;
		}

	});

	exec_button.addEventListener('click', function (evt)
	{
		exec_button.disabled = true;
		//output_textarea.innerHTML = ""; // clear output
		output_editor.setValue(""); // clear output
		//var code = source_textarea.value;
		var code = source_editor.getValue();

		// check if there is no code to be executed
		if (code.length === 0) {
			return;
		}

		NodePlugin.exec(code, function (err)
		{
			//exec_button.disabled = false;
			setTimeout(function () { exec_button.disabled = false; }, 1000);
		});

	}, false);

	clear_button.addEventListener('click', function (evt)
	{
		exec_button.disabled = true;
		source_editor.setValue(""); // clear input
		output_editor.setValue(""); // clear output
	}, false);

	var xhr = new XMLHttpRequest();
	xhr.addEventListener('readystatechange', function (event)
	{
		if ((this.readyState == 4) && ((this.status == 200) || (this.status == 0)))
		{
			//source_textarea.innerHTML = this.responseText;
			source_editor.setValue("");
			source_editor.insert(this.responseText);
		}
	});
	xhr.open("GET", "default.js", true);
	xhr.send();

	// exec_button.disabled = true;
	document.addEventListener('deviceready', function ()
	{
		NodePlugin.loop(
			function (str)
			{
				//output_textarea.innerHTML += str;
				output_editor.insert(str);
			},
			function (str)
			{
				//output_textarea.innerHTML += str;
				output_editor.insert(str);
			}
		);
		//exec_button.disabled = false;
		setTimeout(function () { exec_button.disabled = false; }, 1000);
	}, false);
}

