#include <argpar/argpar.h>

#include <iostream>

int main(int argc, char* argv[])
{
	bool has_version;
	std::string format;
	std::vector<std::string> libs;
	bool has_verbose;
	bool has_help;

	argpar::arg_parser parser;
	parser.option({ "V", "version" }, "Prints out version and exits successfully", &has_version); // optional option
	parser.option({ "f", "format" }, "Sets format for the output.") // mandatory option
		.string_par("FORMAT", &format); // mandatory parameter
	parser.option({ "v", "verbose" }, "Enables verbose output.", &has_verbose);
	parser.option({ "help" }, "Prints out usage and exits successfully", &has_help);

	// other use cases
	bool do_magic;
	int magic_level;
	parser.option({ "optional-option-with-optional-parameter" }, "Does something mega useful.", &do_magic)
		.integer_par("MAGIC_LEVEL", &magic_level).between(1, 8).with_default(5); // optional parameter

	try
	{
		parser.parse(&argc, &argv);
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	if (has_help)
	{
		parser.print_help(std::cout);
		return 0;
	}

	std::vector<std::string> args(argv, argv + argc);
	for(auto && arg : args)
	{
	// process args as usual
	}

	std::cout << "Hello from example" << std::endl;
}
