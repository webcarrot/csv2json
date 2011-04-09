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
or
	csv2json --input-file input.csv --output-file output.json --row-sep $'\n' --col-sep ',' --text-sep '"' -l 9000000


Params:
-------
	csv2json params:
	-i
	--input-file   path to input file [required]
	-o
	--output-file  path to output file [default:NULL] [optional] [if not set write output to stdout]
	-r
	--row-sep      row separator [default:$'\n']
	-c
	--col-sep      col separator [default:',']
	-t
	--text-sep     text separator [default:'"']
	-l             how many chars can exist in single cell. DANGEROUS DO NOT SET TO SMALL.
	               Escaped utf8 consume 4 chars extra and special chars 1 char extra. [default:1000000]
	-h
	--help         print help screen
	-v
	--version      print version screen

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
