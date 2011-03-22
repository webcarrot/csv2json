/**
 * 
 * Copyright 2011 Wojciech Wierchoła <admin@webcarrot.pl>
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

const SUCCESS = 0;
const ERROR_BAD_ARG = 1;
const ERROR_INVALID_INPUT_FILE = 2;
const ERROR_INVALID_OUTPUT_FILE = 3;
const ERROR_MEMORY = 4;
const ERROR_READING_FILE = 5;

int main(int argc,char **argv);
int parseFile(char *input_file,char *output_file,char row_separator,char col_separator,char text_separator,int cell_lenght);
int addChar(char * * char_pointer, char  * current_char);
void addCharToCell(char * * char_pointer, char char_value);
void addStringToCell(char * * char_pointer, char string_value[10]);
void writeTo(char * output_string,FILE * output_file_handler);
void help(void);

/**
 * main - read params etc.
 * @param *char -i path to input file [required]
 * @param *char -o path to output file [default:NULL] [if not set write output to stdout]
 * @param char -r row separator [default:'\n']
 * @param char -c col separator [default:',']
 * @param char -t text separator [default:'"']
 * @param int -l how many chars can exist in single cell. DANGEROUS DO NOT SET TO SMALL. Escaped utf8 consume 4 chars extra and special chars 1 char extra. [default:1000000]
 * @param void -h print help screen
 * @return int
 */
int main(int argc,char **argv)
{
	char *input_file = NULL;
	char *output_file = NULL;
	char row_separator = '\n';
	char col_separator = ',';
	char text_separator = '"';
	int cell_lenght = 1000000;
	int c;
	opterr = 0;
	while ((c = getopt (argc, argv, "i:o:c:r:t:l:h")) != -1)
	{
		switch (c)
		{
			case 'h':
			{
				help();
				return SUCCESS;
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
				if (optopt == 'i' || optopt == 'o' || optopt == 'c' || optopt == 'r' || optopt == 't' || optopt == 'l')
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
	if(input_file != NULL && cell_lenght > 2)
	{
		return parseFile(input_file,output_file,row_separator,col_separator,text_separator,cell_lenght);
	}
	else
	{
		fputs ("Option -i required.",stderr);
		help();
		return ERROR_BAD_ARG;
	}
}

/**
 * parseFile - try to load input file to char array and parse it
 * @param *char input_file path to input file
 * @param *char output_file path to output file [default:NULL]
 * @param char row_separator row separator [default:'\n']
 * @param char col_separator col separator [default:',']
 * @param char text_separator text separator [default:'"']
 * @param int cell_lenght how many chars can exist in single cell [default:1000000]
 * @return int
 */
int parseFile(char *input_file,char *output_file,char row_separator,char col_separator,char text_separator,int cell_lenght)
{
	FILE * input_file_handler = NULL, * output_file_handler = NULL;
	long input_file_handler_size;
	char * file_content;
	input_file_handler = fopen ( input_file , "rb" );
	if (input_file_handler==NULL)
	{
		fprintf (stderr, "Cannot open %s file", input_file);
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
		fputs ("Memory error",stderr);
		return ERROR_MEMORY;
	}
	// copy the file into the file_content:
	if (fread(file_content,1,input_file_handler_size,input_file_handler) != input_file_handler_size)
	{
		fputs ("Reading file error",stderr);
		return ERROR_READING_FILE;
	}
	fclose (input_file_handler);
	// outpufile
	if(output_file != NULL)
	{
		output_file_handler = fopen ( output_file , "w" );
		if (output_file_handler==NULL)
		{
			fprintf (stderr, "Cannot write to %s file", output_file);
			return ERROR_INVALID_OUTPUT_FILE;
		}
	}
	long int i=0;
	char cell_content[cell_lenght];
	char *current_char = NULL,*next_char = NULL,*cell_content_char = cell_content;
	short int cell_with_sep = 0;
	short int cell_without_sep = 0;
	short int row_begin_paresed = 0;
	writeTo("[\n",output_file_handler);
	for(;i<=input_file_handler_size;i++)
	{
		current_char = &file_content[i];
		next_char = &file_content[i+1];
		if(cell_with_sep)
		{
			if(*current_char == text_separator && (*next_char == '\0'  || *next_char == row_separator || *next_char == col_separator))
			{
				addCharToCell(&cell_content_char,'"');
				addCharToCell(&cell_content_char,*next_char == col_separator?',':']');
				addCharToCell(&cell_content_char,'\0');
				writeTo(cell_content,output_file_handler);
				cell_content_char=cell_content;
				cell_with_sep = 0;
			}
			else
			{
				i+=addChar(&cell_content_char,current_char);
			}
		}
		else if(cell_without_sep)
		{
			if(*current_char == '\0'  || *current_char == row_separator || *current_char == col_separator)
			{
				addCharToCell(&cell_content_char,'"');
				addCharToCell(&cell_content_char,*current_char == col_separator?',':']');
				addCharToCell(&cell_content_char,'\0');
				writeTo(cell_content,output_file_handler);
				cell_content_char=cell_content;
				cell_without_sep = 0;
				if(*current_char == row_separator)
				{
					row_begin_paresed = 0;
					if(*next_char == '\0' || i+1 == input_file_handler_size)
					{
						break;
					}
					else
					{
						writeTo(",\n",output_file_handler);
					}
				}
			}
			else
			{
				i+=addChar(&cell_content_char,current_char);
			}
		}
		else if(*current_char == text_separator)
		{
			cell_with_sep = 1;
			if(!row_begin_paresed)
			{
				row_begin_paresed = 1;
				addCharToCell(&cell_content_char,'[');
			}
			addCharToCell(&cell_content_char,'"');
		}
		else if(*current_char == row_separator)
		{
			row_begin_paresed = 0;
			if(*next_char == '\0' || i+1 == input_file_handler_size)
			{
				break;
			}
			else
			{
				writeTo(",\n",output_file_handler);
			}
		}
		else if(*current_char == col_separator)
		{
			if(!row_begin_paresed)
			{
				writeTo((*next_char == '\0'  || *next_char == row_separator)?"[\"\"]":"[\"\",",output_file_handler);
				row_begin_paresed = 1;
			}
			else if(*next_char == col_separator)
			{
				writeTo("\"\",",output_file_handler);
			}
			else if(*next_char == '\0'  || *next_char == row_separator)
			{
				writeTo("\"\"]",output_file_handler);
			}
		}
		else
		{
			cell_without_sep = 1;
			if(!row_begin_paresed)
			{
				row_begin_paresed = 1;
				addCharToCell(&cell_content_char,'[');
			}
			addCharToCell(&cell_content_char,'"');
			i+=addChar(&cell_content_char,current_char);
		}
	}
	writeTo("\n]\n",output_file_handler);
	if(output_file_handler != NULL)
	{
		fclose (output_file_handler);
	}
	free (file_content);
	return SUCCESS;
}


/**
 * addChar - add char to chars array
 * @param **char char_pointer pointer to pointer to chars array
 * @param *char current_char pointer to char array created from input file
 * @return int - how many chars should be skipped
 */
int addChar(char * * char_pointer, char * current_char)
{
	switch(*current_char)
	{
		case '\\':
		{
			addCharToCell(char_pointer,'\\');
			addCharToCell(char_pointer,'\\');
			return 0;
		}
		case '\n':
		{
			addCharToCell(char_pointer,'\\');
			addCharToCell(char_pointer,'n');
			return 0;
		}
		case '\t':
		{
			addCharToCell(char_pointer,'\\');
			addCharToCell(char_pointer,'t');
			return 0;
		}
		case '\r':
		{
			addCharToCell(char_pointer,'\\');
			addCharToCell(char_pointer,'r');
			return 0;
		}
		case '\b':
		{
			addCharToCell(char_pointer,'\\');
			addCharToCell(char_pointer,'b');
			return 0;
		}
		case '\f':
		{
			addCharToCell(char_pointer,'\\');
			addCharToCell(char_pointer,'f');
			return 0;
		}
		case '/':
		case '"':
		{
			addCharToCell(char_pointer,'\\');
			addCharToCell(char_pointer,*current_char);
			return 0;
		}
		default:
		{
			/**
			* @todo add support for other chars
			*/
			break;
		}
	}
	if ((*current_char >= 0x20) && (*current_char <= 0x7F))
	{
		addCharToCell(char_pointer,*current_char);
		return 0;
	}
	else if ((*current_char & 0xE0) == 0xC0)
	{
		char utf16[7];
		sprintf(
			utf16,
			"\\u%02X%02X",
			(0x07 & *current_char >> 2),
			((0xC0 & *current_char << 6) | (0x3F & *(current_char+1)))
		);
		addStringToCell(char_pointer,utf16);
		return 1;
	}
	else if((*current_char & 0xF0) == 0xE0)
	{
		char utf16[7];
		sprintf(
			utf16,
			"\\u%02X%02X",
			((0xF0 & *current_char << 4) | (0x0F & *(current_char+1) >> 2)),
			((0xC0 & *(current_char+1) << 6) | (0x7F & *(current_char+2)))
		);
		addStringToCell(char_pointer,utf16);
		return 2;
	}
	else if((*current_char & 0xF8) == 0xF0)
	{
		/**
		 * @todo add support for utf?? chars
		 */
		return 3;
	}
	else if((*current_char & 0xFC) == 0xF8)
	{
		/**
		 * @todo add support for utf?? chars
		 */
		return 4;
	}
	else if((*current_char & 0xFE) == 0xFC)
	{
		/**
		 * @todo add support for utf?? chars
		 */
		return 5;
	}
	return 0;
}

/**
 * addCharToCell - add char to chars array
 * @param **char char_pointer pointer to pointer to chars array
 * @param char char_value char to add to char_pointer array
 * @return void
 */
void addCharToCell(char * * char_pointer, char char_value)
{
	(*(*char_pointer)) =char_value;
	(*char_pointer)++;
}

/**
 * addStringToCell - add multiple chars to chars array
 * @param **char char_pointer pointer to pointer to chars array
 * @param char string_value chars array to add to char_pointer array
 * @return void
 */
void addStringToCell(char * * char_pointer, char string_value[20])
{
	char * c = string_value;
	while( *c != '\0')
	{
		addCharToCell(char_pointer,*c);
		c++;
	}
}

/**
 * writeTo - write chars to file or stdout
 * @param *char output_string pointer to chars array
 * @param *FILE output_file_handler handler to output file
 * @return void
 */
void writeTo(char * output_string,FILE * output_file_handler)
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
 * @return void
 */
void help(void)
{
	fputs("ccsv2json params:\
	\n\t-i path to input file [required]\
	\n\t-o path to output file [default:NULL] [optional] [if not set write output to stdout]\
	\n\t-r row separator [default:'\\n']\
	\n\t-c col separator [default:',']\
	\n\t-t text separator [default:'\"']\
	\n\t-l how many chars can exist in single cell. DANGEROUS DO NOT SET TO SMALL. Escaped utf8 consume 4 chars extra and special chars 1 char extra. [default:1000000]\
	\n\t-h print help screen\n",stdout);
	return;
}
