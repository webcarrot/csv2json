# csv2json

## Description:

Writen in C, CSV file to JSON file/string converter with utf8 support.

Can generate array of arrays or array of objects.

## Simple usage:

    csv2json -i input.csv > output.json

with keys:

    csv2json -i input.csv -k 999 > output.json

## Complex usage:

    csv2json -e -i input.csv -o output.json -r $'\n' -c ',' -t '"' -l 9000000 -k 99

or

    csv2json --escape --input-file input.csv --output-file output.json --row-sep $'\n' --col-sep ',' --text-sep '"' --cell-length 9000000 --keys 99

## Params:

```
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
-l
--cell-length  how many chars can exist in single cell. DO NOT SET TO SMALL.
               Escaped utf8 consume 4 chars extra and special chars 1 char extra. [default:1000000]
-k
--keys         set maximum keys number and use first row values as keys for values [default:0]
-e
--escape       escape UTF-8 (some 'long' chars are not supported) [flag:default:false]
-h
--help         print help screen
-v
--version      print version screen
```

## TODO:

### Add support for utf16/32
