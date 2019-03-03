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
	parser.opt('V', "version", "Prints out version and exits successfully", &has_version);
	parser.opt('f', "format", "Sets format for the output.")
		.string("FORMAT", &format).with_default("utc");
	parser.opt('v', "verbose", "Enables verbose output.", &has_verbose);
	parser.opt('h', "help", "Prints out usage and exits successfully", &has_help);

	// other options (not related to the example from assignment)
	parser.opt('l', "link", "Adds libraries to link")
		.string_list("LIB", &libs);
	parser.opt(0, "long-only-option", "An option that can be set only via long option")
		.conflicts({ "conflicting-option" });

	// ...

	if (has_help)
	{
		parser.print_help(std::cout);
		return 0;
	}

	std::cout << "Hello from example" << std::endl;
}
