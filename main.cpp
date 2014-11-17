#include "ProgramOption.h"

#include <iostream>
using namespace std;
using namespace program_option;

namespace config
{
	string operation;
	string outfile ;
	vector<string> infiles;
	int max_size ;
	bool read_stdin;
	bool verbose ;

	void show_value() {
#define show(x) (cerr << #x ": " << x << endl)
		show(operation);
		show(outfile);
		cerr << "infiles: " ;
		for (int i=0;i<infiles.size();++i) cerr << (i==0?"":", ") << infiles[i] ; cerr << endl;
		show(max_size);
		show(read_stdin);
		show(verbose);
	}
}

/*
	usage: do_what_ever [options] operation [outfile] [ infile ... ]
	  operation: some operations
	  outfile: output file (stdout if no specified)
	  infile: input file(s) (stdin if no specified)

	do whatever to the infile (or stdin), and write to outfile (or stdout)

	options:
		--also-read-stdin     read stdin as an additional input (if infile specified)
		--max-size, -M <size> max output size
		--help, -h            show help message
		 -v                   verbose
		
		--bomb, -b            shutdown system (invisible)
        --help2               show level2 help message
*/

void bomb()
{
	cerr << "System is going to shutdown ... " << endl;
	exit(1);
}

int main(int argc, char* argv[])
{
	ProgramOption po(argv[0], "do whatever to the infile (or stdin), and write to outfile (or stdout)");
	using namespace config;

	po.addOption(Option(operation, load<string>)
		.is_default_arg("operation")
		.description("some operations\nanother line\n")
		);
	po.addOption(Option(po.invoke_set_group())
		.is_default_arg("mode")
		.is_optional()
		.description("specify mode")
		);

	po.addOption(Option(outfile, load<string>)
		.is_default_arg("outfile")
		.is_optional()
		.description("output file (stdout if no specified)")
		);
	po.addOption(Option(infiles, load2< vector<string> >)
		.is_default_arg("infile")
		.is_optional()
		.is_arg_list()
		.description("input file(s) (stdin if no specified)")
		);
	po.addOption(Option(po.invoke_help(cerr)).keys("help", 'h').no_arg().description("show help message"));

	po.addOption(Option(new ValueSetter<bool>(read_stdin, true))
		.long_key("also-read-stdin")
		.no_arg()
		.description("read stdin as an additional input\n(if infile specified)")
		);

	po.addOption(Option(new ValueSetter<bool>(verbose, true))
		.short_key('v')
		.no_arg()
		.description("verbose")
		);
	po.addOption(Option(max_size, load<int>)
		.keys("max-size", 'M')
		.name("size")
		.description("max output size")
		);
	po.addOption(Option(newMethodCaller(bomb))
		.keys("bomb", 'b')
		.invisible()
		.description("shutdown system (invisible)")
		);

	po.addOption(Option(po.invoke_help(cerr, 2)).long_key("help2").no_arg().description("show level2 help message").help_level(2));

	po.addOption(Option(new ValueSetter<int>(max_size, -1))
		.group("dummy")
		.no_arg()
		.short_key('M')
		.description("set size to -1")
		);

	cerr << po.getError() << endl;


	if (!po.parse(argc, argv)) {
		cerr << po.getError() << endl;
		cerr << po.usage(0) ;
		exit(1);
	}

	show_value();

	return 0;
}
