/**
 * Copyright (c) Flyover Games, LLC
 */

var NodePlugin = {};

NodePlugin.loop = function (stdout_write, stderr_write)
{
	function success (value)
	{
		if (typeof(value) === 'string')
		{
			stdout_write(value);
		}
	}

	function error (value)
	{
		if (typeof(value) === 'string')
		{
			stderr_write(value);
		}
	}

	cordova.exec(success, error, 'NodePlugin', 'loop', []);
}

NodePlugin.done = function ()
{
	cordova.exec(success, error, 'NodePlugin', 'done', []);
}

NodePlugin.exec = function (code, callback)
{
	function success (value)
	{
		callback(value);
	}

	function error (value)
	{
		callback(value);
	}

	cordova.exec(success, error, 'NodePlugin', 'exec', [ code ]);
}

