console.log("Hello, World!");

var isVisible = function (value, index, array)
{
	return value[0] !== '.';
};

var printName = function (value, index, array)
{
	console.log(value);
};

var fs = require('fs');
fs.readdir("/", function (err, array)
{
	if (err) { return console.log(err); }
	array.filter(isVisible).forEach(printName);
});

