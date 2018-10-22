/**
 *
 * Copyright (C) 2011 Wojciech Wierchoła <admin@webcarrot.pl>
 *
 * Description: writen in C, CSV file to JSON file/string converter
 * with utf8 support.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

int isprint ( int c );

// return codes
const int SUCCESS = 0;
const int ERROR_BAD_ARG = 1;
const int ERROR_INVALID_INPUT_FILE = 2;
const int ERROR_INVALID_OUTPUT_FILE = 3;
const int ERROR_MEMORY = 4;
const int ERROR_READING_FILE = 5;
const int ERROR_CELL_LENGTH = 6;
const int ERROR_ENCODING = 7;

// version
const char PROGRAM_VERSION[32] = "0.5";

// main functions
int main(int argc,char **argv);
int parseFile(char *input_file,char *output_file,char row_separator,char col_separator,char text_separator,int cell_lenght,short int keys_number, unsigned char escape);

// char functions
int addChar(char * * char_pointer, char  * current_char, char * cell_content_end, long unsigned int to_read, unsigned char escape);
void addCharToCell(char * * char_pointer, char char_value, char * cell_content_end);
void addStringToCell(char * * char_pointer, char string_value[10], char * cell_content_end);

// key functions
void addUnknownKeyCol(char * * keys, int col, int cell_lenght);
void writeCellToKey(char * output_string, char * * key);

// cell functions
void writeCellTo(char * output_string, char * key_string,FILE * output_file_handler);
void writeTo(char * output_string,FILE * output_file_handler);

// other
void help(void);
void version(void);

/**
 * main - read params etc.
 */
int main(int argc,char **argv)
{
	// set default params values
	char *input_file = NULL;
	char *output_file = NULL;
	char row_separator = '\n';
	char col_separator = ',';
	char text_separator = '"';
	int cell_lenght = 1000000;
	short int keys_number = 0;
  unsigned char escape = 0;
	// get params
	int c;
	int option_index = 0;
	opterr = 0;
	static struct option long_options[] =
	{
		{"input-file",   required_argument, 0, 'i'},
		{"output-file",  required_argument, 0, 'o'},
		{"row-sep",      required_argument, 0, 'r'},
		{"col-sep",      required_argument, 0, 'c'},
		{"text-sep",     required_argument, 0, 't'},
		{"cell-length",  required_argument, 0, 'l'},
 		{"keys",         required_argument, 0, 'k'},
    {"escape",       no_argument,       0, 'e'},
		{"help",         no_argument,       0, 'h'},
		{"version",      no_argument,       0, 'v'},
		{0, 0, 0, 0}
	};
	while ((c = getopt_long(argc, argv, "i:o:c:r:t:l:k:ehv",long_options, &option_index)) != -1)
	{
		switch (c)
		{
			case 'h':
			{
				help();
				return SUCCESS;
			}
			case 'v':
			{
				version();
				return SUCCESS;
			}
      case 'e':
			{
				escape = 1;
        break;
			}
			case 'k':
			{
				keys_number = atoi(optarg);
				break;
			}
			case 'i':
			{
				input_file = optarg;
				break;
			}
			case 'o':
			{
				output_file = optarg;
				break;
			}
			case 'c':
			{
				col_separator = *optarg;
				break;
			}
			case 'r':
			{
				row_separator = *optarg;
				break;
			}
			case 't':
			{
				text_separator = *optarg;
				break;
			}
			case 'l':
			{
				cell_lenght = atoi(optarg);
				break;
			}
			case '?':
			{
				if (optopt == 'i' || optopt == 'o' || optopt == 'c' || optopt == 'r' || optopt == 't' || optopt == 'l' || optopt == 'k')
				{
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				}
				else if (isprint (optopt))
				{
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				}
				else
				{
					fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
				}
				help();
				return ERROR_BAD_ARG;
			}
			default:
			{
				abort();
			}
		}
	}
	// check params
	if(input_file != NULL && cell_lenght > 2)
	{
		// parse file
		return parseFile(input_file,output_file,row_separator,col_separator,text_separator,cell_lenght,keys_number,escape);
	}
	else
	{
		// print error
		if(input_file == NULL)
		{
			fputs ("Option -i required.\n",stderr);
		}
		if(cell_lenght < 3)
		{
			fputs ("To low -l/--cell-length parameter value.\n",stderr);
		}
		help();
		return ERROR_BAD_ARG;
	}
}

/**
 * parseFile - try to load input file to char array and parse it
 */
int parseFile(char *input_file,char *output_file,char row_separator,char col_separator,char text_separator,int cell_lenght,short int keys_number, unsigned char escape)
{
	// input file
	FILE * input_file_handler = NULL, * output_file_handler = NULL;
	long input_file_handler_size;
	char * file_content;
	input_file_handler = fopen ( input_file , "rb" );
	if (input_file_handler==NULL)
	{
		fprintf (stderr, "Cannot open %s file\n", input_file);
		return ERROR_INVALID_INPUT_FILE;
	}
	// obtain file size:
	fseek (input_file_handler , 0 , SEEK_END);
	input_file_handler_size = ftell (input_file_handler);
	rewind (input_file_handler);
	// allocate memory to contain the whole file:
	file_content = (char*) malloc (sizeof(char)*input_file_handler_size);
	if (file_content == NULL)
	{
		fputs ("Memory error\n",stderr);
		return ERROR_MEMORY;
	}
	// copy the file into the file_content:
	if (fread(file_content,1,input_file_handler_size,input_file_handler) != input_file_handler_size)
	{
		fputs ("Reading file error\n",stderr);
		return ERROR_READING_FILE;
	}
	fclose (input_file_handler);
  // detect utf16-32
  if (
    input_file_handler_size > 3 &&
    (
      // standard utf16/32 headers
      (
        ((unsigned char)file_content[0]) == 0xFF &&
        ((unsigned char)file_content[1]) == 0xFE
      ) ||
      // try to detect LE/BE mode
      ((unsigned char)file_content[0]) == 0x00 ||
      ((unsigned char)file_content[1]) == 0x00 ||
      ((unsigned char)file_content[2]) == 0x00
    )
  ) {
    fputs ("Input encoding error - look like UTF-16 or UTF-32\n", stderr);
    return ERROR_ENCODING;
  }

	// output file
	if(output_file != NULL)
	{
		output_file_handler = fopen ( output_file , "w" );
		if (output_file_handler==NULL)
		{
			fprintf (stderr, "Cannot write to %s file\n", output_file);
			return ERROR_INVALID_OUTPUT_FILE;
		}
	}
	// parse data
	// keys ?
	char right_row_delimiter[2];
	char left_row_delimiter[2];
	right_row_delimiter[1] = '\0';
	left_row_delimiter[1] = '\0';
	char * keys[keys_number];
	int col = 0;
	short int keys_parsed = 0;
	int known_keys = 0;
	if(keys_number)
	{
		right_row_delimiter[0] = '}';
		left_row_delimiter[0] = '{';
	}
	else
	{
		right_row_delimiter[0] = ']';
		left_row_delimiter[0] = '[';
	}
	// cell content, rows
	long unsigned int i=0;
  long unsigned int to_read=0;
	char cell_content[cell_lenght];
	char *current_char = NULL,*next_char = NULL,*cell_content_char = cell_content,*cell_content_end=&(cell_content[cell_lenght-1]);
	short int cell_with_sep = 0;
	short int cell_without_sep = 0;
	short int row_begin_paresed = 0;
	char null = '\0';
	// parse
	writeTo("[\n",output_file_handler);
	for(;i<=input_file_handler_size;i++)
	{
    to_read = i < input_file_handler_size ? input_file_handler_size - i : 0;
		current_char = to_read > 0 ? &file_content[i] : &null;
		next_char = to_read > 1 ? &file_content[i+1] : &null;
		if(cell_with_sep)
		{
			if(*current_char == text_separator && (*next_char == null  || *next_char == row_separator || *next_char == col_separator))
			{
				addCharToCell(&cell_content_char,null,cell_content_end);
				if(keys_number && !keys_parsed)
				{
					writeCellToKey(cell_content,&(keys[col]));
					known_keys++;
				}
				else
				{
					if(keys_number)
					{
						if(col >= known_keys)
						{
							addUnknownKeyCol(keys,col,cell_lenght);
							known_keys++;
						}
						writeCellTo(cell_content,keys[col],output_file_handler);
					}
					else
					{
						writeCellTo(cell_content,NULL,output_file_handler);
					}
				}
				col++;
				cell_content_char=cell_content;
				cell_with_sep = 0;
				if(*next_char == col_separator && ( !keys_number || keys_parsed ))
				{
					writeTo(",",output_file_handler);
				}
			}
			else
			{
				i+=addChar(&cell_content_char,current_char,cell_content_end,to_read,escape);
			}
		}
		else if(cell_without_sep)
		{
			if(*current_char == null  || *current_char == row_separator || *current_char == col_separator)
			{
				addCharToCell(&cell_content_char,null,cell_content_end);
				if(keys_number && !keys_parsed)
				{
					writeCellToKey(cell_content,&(keys[col]));
					known_keys++;
				}
				else
				{
					if(keys_number)
					{
						if(col >= known_keys)
						{
							addUnknownKeyCol(keys,col,cell_lenght);
							known_keys++;
						}
						writeCellTo(cell_content,keys[col],output_file_handler);
					}
					else
					{
						writeCellTo(cell_content,NULL,output_file_handler);
					}
				}
				col++;
				cell_content_char = cell_content;
				cell_without_sep = 0;
				if(*current_char == col_separator && (!keys_number || keys_parsed))
				{
					writeTo(",",output_file_handler);
				}
				i--;
			}
			else
			{
				i+=addChar(&cell_content_char,current_char,cell_content_end,to_read,escape);
			}
		}
		else if(*current_char == text_separator)
		{
			cell_with_sep = 1;
			if(!row_begin_paresed)
			{
				row_begin_paresed = 1;
				if(!keys_number || keys_parsed)
				{
					writeTo(left_row_delimiter,output_file_handler);
				}
			}
		}
		else if(*current_char == row_separator || *current_char == null)
		{
			if(*next_char == row_separator)
			{
				continue;
			}
			if(row_begin_paresed)
			{
				row_begin_paresed = 0;
				col = 0;
				if(*next_char == null || i+1 >= input_file_handler_size)
				{
					if(!keys_number || keys_parsed)
					{
						writeTo(right_row_delimiter,output_file_handler);
					}
					break;
				}
				else if(keys_number && !keys_parsed)
				{
					keys_parsed = 1;
				}
				else
				{
					writeTo(right_row_delimiter,output_file_handler);
					writeTo(",\n",output_file_handler);
				}
			}
		}
		else if(*current_char == col_separator)
		{
			if(!row_begin_paresed && (!keys_number || keys_parsed))
			{
				writeTo(left_row_delimiter,output_file_handler);
			}
			if(!row_begin_paresed || *next_char == col_separator || *next_char == null  || *next_char == row_separator)
			{
				if(!row_begin_paresed)
				{
					i--;
				}
				if(keys_number)
				{
					if(keys_parsed)
					{
						if(col >= known_keys)
						{
							addUnknownKeyCol(keys,col,cell_lenght);
							known_keys++;
						}
						writeCellTo("",keys[col],output_file_handler);
					}
					else
					{
						row_begin_paresed = 1;
						addUnknownKeyCol(keys,col,cell_lenght);
						known_keys++;
						col++;
						continue;
					}
				}
				else
				{
					writeCellTo("",NULL,output_file_handler);
				}
				col++;
				if(!row_begin_paresed || *next_char == col_separator)
				{
					row_begin_paresed = 1;
					writeTo(",",output_file_handler);
				}
			}
		}
		else
		{
			cell_without_sep = 1;
			if(!row_begin_paresed)
			{
				row_begin_paresed = 1;
				if(!keys_number || keys_parsed)
				{
					writeTo(left_row_delimiter,output_file_handler);
				}
			}
			i+=addChar(&cell_content_char,current_char,cell_content_end,to_read,escape);
		}
	}
	writeTo("\n]\n",output_file_handler);
	// parse end
	if(output_file_handler != NULL)
	{
		// close output file
		fclose (output_file_handler);
	}
	// free memory
	for(i=0;i<known_keys;i++)
	{
		free(keys[i]);
	}
	free (file_content);
	return SUCCESS;
}

/**
 * addChar - add char to chars array
 */
int addChar(char * * char_pointer, char * current_char, char * cell_content_end, long unsigned int to_read, unsigned char escape)
{
	switch(*current_char)
	{
		case '\\':
		{
			addCharToCell(char_pointer,'\\',cell_content_end);
			addCharToCell(char_pointer,'\\',cell_content_end);
			return 0;
		}
		case '\n':
		{
			addCharToCell(char_pointer,'\\',cell_content_end);
			addCharToCell(char_pointer,'n',cell_content_end);
			return 0;
		}
		case '\t':
		{
			addCharToCell(char_pointer,'\\',cell_content_end);
			addCharToCell(char_pointer,'t',cell_content_end);
			return 0;
		}
		case '\r':
		{
			addCharToCell(char_pointer,'\\',cell_content_end);
			addCharToCell(char_pointer,'r',cell_content_end);
			return 0;
		}
		case '\b':
		{
			addCharToCell(char_pointer,'\\',cell_content_end);
			addCharToCell(char_pointer,'b',cell_content_end);
			return 0;
		}
		case '\f':
		{
			addCharToCell(char_pointer,'\\',cell_content_end);
			addCharToCell(char_pointer,'f',cell_content_end);
			return 0;
		}
		case '/':
		case '"':
		{
			addCharToCell(char_pointer,'\\',cell_content_end);
			addCharToCell(char_pointer,*current_char,cell_content_end);
			return 0;
		}
		default:
		{
			break;
		}
	}
	if ((*current_char >= 0x20) && (*current_char <= 0x7F))
	{
		addCharToCell(char_pointer,*current_char,cell_content_end);
		return 0;
	}
  unsigned char u8 = (unsigned char)(*current_char);
	if ((to_read > 1) && (u8 & 0xE0) == 0xC0)
	{
    if (escape == 0) {
      addCharToCell(char_pointer,*current_char,cell_content_end);
      addCharToCell(char_pointer,*(current_char+1),cell_content_end);
    } else {
  		char utf8[7];
  		sprintf(
  			utf8,
  			"\\u%02X%02X",
  			(0x07 & *current_char >> 2),
  			((0xC0 & *current_char << 6) | (0x3F & *(current_char+1)))
  		);
  		addStringToCell(char_pointer,utf8,cell_content_end);
    }
    return 1;
	}
	else if((to_read > 2) && (u8 & 0xF0) == 0xE0)
	{
    if (escape == 0) {
      addCharToCell(char_pointer,*current_char,cell_content_end);
      addCharToCell(char_pointer,*(current_char+1),cell_content_end);
      addCharToCell(char_pointer,*(current_char+2),cell_content_end);
    } else {
  		char utf8[7];
  		sprintf(
  			utf8,
  			"\\u%02X%02X",
  			((0xF0 & *current_char << 4) | (0x0F & *(current_char+1) >> 2)),
  			((0xC0 & *(current_char+1) << 6) | (0x7F & *(current_char+2)))
  		);
  		addStringToCell(char_pointer,utf8,cell_content_end);
    }
    return 2;
	}
	else if((to_read > 3) && (u8 & 0xF8) == 0xF0)
	{
    if (escape == 0) {
      addCharToCell(char_pointer,*current_char,cell_content_end);
      addCharToCell(char_pointer,*(current_char+1),cell_content_end);
      addCharToCell(char_pointer,*(current_char+2),cell_content_end);
      addCharToCell(char_pointer,*(current_char+3),cell_content_end);
    }
		return 3;
	}
	else if((to_read > 4) && (u8 & 0xFC) == 0xF8)
	{
    if (escape == 0) {
      addCharToCell(char_pointer,*current_char,cell_content_end);
      addCharToCell(char_pointer,*(current_char+1),cell_content_end);
      addCharToCell(char_pointer,*(current_char+2),cell_content_end);
      addCharToCell(char_pointer,*(current_char+3),cell_content_end);
      addCharToCell(char_pointer,*(current_char+4),cell_content_end);
    }
		return 4;
	}
	else if((to_read > 5) && (u8 & 0xFE) == 0xFC)
	{
    if (escape == 0) {
      addCharToCell(char_pointer,*current_char,cell_content_end);
      addCharToCell(char_pointer,*(current_char+1),cell_content_end);
      addCharToCell(char_pointer,*(current_char+2),cell_content_end);
      addCharToCell(char_pointer,*(current_char+3),cell_content_end);
      addCharToCell(char_pointer,*(current_char+4),cell_content_end);
      addCharToCell(char_pointer,*(current_char+5),cell_content_end);
    }
		return 5;
	}
  return 0;
}

/**
 * addCharToCell - add char to chars array
 */
void addCharToCell(char * * char_pointer, char char_value, char * cell_content_end)
{
	(*(*char_pointer)) = char_value;
	if(*char_pointer == cell_content_end)
	{
		fputs ("\nTo low -l/--cell-length parameter value. Output is corrupted!\n",stderr);
		exit(ERROR_CELL_LENGTH);
	}
	(*char_pointer)++;
	return;
}

/**
 * addStringToCell - add multiple chars to chars array
 */
void addStringToCell(char * * char_pointer, char string_value[20], char * cell_content_end)
{
	char * c = string_value;
	while( *c != '\0')
	{
		addCharToCell(char_pointer,*c,cell_content_end);
		c++;
	}
	return;
}

/**
 * addUnknownKeyCol - write "unknown-xx" to keys array
 */
void addUnknownKeyCol(char * * keys, int col, int cell_lenght)
{
	char value[cell_lenght];
	sprintf(value,"unknown-%d",col+1);
	writeCellToKey(value,&keys[col]);
}


/**
 * writeCellToKey - write cell value to keys array
 */
void writeCellToKey(char * output_string, char * * key)
{
	*key = (char*) malloc(sizeof(char)*(strlen(output_string)+1)); // MS windows + 1?
	sprintf(*key,"%s",output_string);
}

/**
 * writeCellTo - write cell value to file or stdout
 */
void writeCellTo(char * output_string,char * key_string, FILE * output_file_handler)
{
	if(key_string != NULL)
	{
		writeTo("\"",output_file_handler);
		writeTo(key_string,output_file_handler);
		writeTo("\":",output_file_handler);
	}
	writeTo("\"",output_file_handler);
	writeTo(output_string,output_file_handler);
	writeTo("\"",output_file_handler);
	return;
}

/**
 * writeTo - write chars to file or stdout
 */
void writeTo(char * output_string, FILE * output_file_handler)
{
	if(output_file_handler != NULL)
	{
		fwrite(output_string, 1, strlen(output_string), output_file_handler);
	}
	else
	{
		printf("%s",output_string);
	}
	return;
}

/**
 * help - print help info
 */
void help(void)
{
	fputs("csv2json params:\
\n-i\
\n--input-file   path to input file [required]\
\n-o\
\n--output-file  path to output file [default:NULL] [optional] [if not set write output to stdout]\
\n-r\
\n--row-sep      row separator [default:$'\\n']\
\n-c\
\n--col-sep      col separator [default:',']\
\n-t\
\n--text-sep     text separator [default:'\"']\
\n-l\
\n--cell-length  how many chars can exist in single cell. DO NOT SET TO SMALL.\
\n               Escaped utf8 consume 4 chars extra and special chars 1 char extra. [default:1000000]\
\n-k\
\n--keys         set maximum keys number and use first row values as keys for values [default:0]\
\n-e\
\n--escape       escape UTF-8 (some 'long' chars are not supported) [flag:default:false]\
\n-h\
\n--help         print help screen\
\n-v\
\n--version      print version screen\n\n",stdout);
	return;
}

/**
 * version - print version info
 */
void version(void)
{
	printf("\
cvs2json version %s\
\nCopyright (C) 2011 Wojciech Wierchola <admin@webcarrot.pl>.\
\n\nThis program is free software; you can redistribute it and/or\
\nmodify it under the terms of the GNU General Public License as\
\npublished by the Free Software Foundation; either version 3 of\
\nthe License, or (at your option) any later version.\
\n\nThis program is distributed in the hope that it will be useful,\
\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\
\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\
\nGNU General Public License for more details.\
\n\nYou should have received a copy of the GNU General Public License\
\nalong with this program.  If not, see <http://www.gnu.org/licenses/>.\
\n\n",PROGRAM_VERSION);
	return;
}
