ProgramOption
=======

An library to support program option parsing, with auto generated help message.<br/>
It was designed to avoid write code to assign variable, parse string to particular type, sync the help message and what the code written, ... etc.

## Format design
This library the options into three kinds of option, "default", "long", and "short", which is come from the existed binary.

### default option
The default option is which has no name specified, its meanning is come from its order in argument list.<br/>

    grep pattern file

- the "pattern" and "file" present without specified its meanning, it specified by its order in argument list.
- default option may be optional, or listed. <br/>
- for example, "file" in grep is optional, and listed, you can specified no flie to grep, or multiple files.

### long option
The long option is who can specified its meanning by two hyphen ("--") and its name.<br/>

    grep ??? ???

### short option
The short option is who can specified its meanning by single hyphen ("-") and its brief.<br/>

    grep -ni pattern file

- the "-n" to show result with line number, and "-i" to match caseless, we can see that the short option support "compress", it allows to set multiple option togeter with a hyphen.

## Features
- Handing different properties with each type of option.
- Auto generating good-look help messages.

### default option
three kinds of default option as following:
- normal default option, which is necessary to run this program
 - e.g. "pattern" string in grep.
- optional default option, which is optional to run this program
 - e.g. "file" in grep, without "file", it grep pattern from stdin.
- listed default option, which can store multiple value, it may be optional or not.
 - e.g. "file" in grep, specify multiple file to grep is OK.
 - you can only have one listed default option

### long option
the long option (long-opt) may have argument following or not.<br/>
for long-opt with argument, it can specified by:
- --option argument
- --option=argument

### short option
the short option (short-opt) may have argument following or not.<br/>
for short-opt with argument, it can specified by:
- -o argument <br/>
- -o=argument <br/>
- -oargument

for short-opt without argument, it can compress and share on hyphen
- -ABC

in compressed short-opt, the last opt may have argument
- -ABCo argument
- -ABCo=argument
- -ABCoargument

### help level
sometimes we don't want to disclose all option for users. <br/>
we can set help_level to each options. <br/>
call usage(level) only return the options with help_level smaller than "level"

### auto generation of help message
the auto generated help message will looks like:

	Usage: <program_name> [options] normal_defaults [optional_defualts] [optional_listed_default ... ]
	"description of this program"

	  <default_option_name>: <description of default_option>
	  <other>              : <description of default_option>
	  ...
	options:
	  --long_opt_without_short_name  <description of option>
	  --opt_w_two_name, -o           <description of option>
	   -s <argurment_for_short_opt>  <description of option>
	  ...
- The first line show the usage with default options, and "[options]" if other option exists.
- The second line is description of this program, which is given by developer.
- following the description of each default option (if non-empty), they align to the longest name of default option.
- The "options" session start by a line with "options:".
- each line shows format to use the option, for options with both long-name and short-name, it show first long-name then following short-name with a comma (",") and space to separate them.
- for options with argument, the name will be showed in argument field, or arg if not specified.
- for the option with invisible property, it will be showed in nowhere.

### not implement yet
- variable overwrite handling 
- group options
- ...
