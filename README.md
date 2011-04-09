csv2json
========

Description:
------------
Writen in C, CSV file to JSON file/string converter with utf8 support ( utf16/32 is not supported).

Simple usage:
-------------
csv2json -i input.csv > output.json

Complex usage:
------------
csv2json -i input.csv -o output.json -r $'\n' -c ',' -t '"' -l 9000000

Params:
-------
	-i path to input file [required]
	-o path to output file [default:NULL] [optional] [if not set write output to stdout]
	-r row separator [default:$'\n']
	-c col separator [default:',']
	-t text separator [default:'"']
	-l how many chars can exist in single cell. DANGEROUS DO NOT SET TO SMALL. Escaped utf8 consume 4 chars extra and special chars 1 char extra. [default:1000000]
	-h print help screen

TODO:
-----
### Add better support for invalid CSV files:
Now for a CSV file with content such as:
	"bla bla bla","bla bla bla"
	","bla bla"
returns:
	[
	["bla bla bla","bla bla bla"],
	[",\"bla bla"]
	]
should be:
	[
	["bla bla bla","bla bla bla\"\n","bla bla"]
	]
Or for CSV content like:
	"bla bla bla","bla bla bla",","bla bla"
returns:
	[
	["bla bla bla","bla bla bla",",\"bla bla"]
	]
should be:
	[
	["bla bla bla","bla bla bla\",","bla bla"]
	]
### Add support for header row
