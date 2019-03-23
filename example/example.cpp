#include <argpar/argpar.h>

#include <iostream>
#include <vector>
#include <optional>
#include <memory>

int main(int argc, char* argv[])
{
	bool has_version;
	std::string format;
	std::vector<std::string> libs;
	std::string command;
	std::vector<std::string> commandArgs;
	bool has_verbose;
	bool has_help;

	argpar::parser parser;
	parser.option({ "V", "version" }, "Prints out version and exits successfully", &has_version); // optional option
	parser.option({ "f", "format" }, "Sets format for the output.") // mandatory option
		.string_val("FORMAT", &format); // mandatory parameter
	parser.option({ "v", "verbose" }, "Enables verbose output.", &has_verbose);
	parser.option({ "help" }, "Prints out usage and exits successfully", &has_help);
	parser.argument().string_val("command", &command);
	parser.argument_list().string_val("arguments", &commandArgs);

	// other use cases
	bool do_magic;
	int magic_level;
	parser.option({ "optional-option-with-optional-parameter" }, "Does something mega useful.", &do_magic)
		.int_val("MAGIC_LEVEL", &magic_level).between(1, 8).with_default(5); // optional parameter

	class custom_config : public argpar::defaultable<custom_config>
	{
	public:
		using value_type = std::unique_ptr<int>;

		value_type parse(char const * arg) const
		{
			return std::make_unique<int>(1);
		}
	};

	std::unique_ptr<int> my_ptr;

	parser.argument().custom_val<custom_config>("myCustomConf", &my_ptr);

	try
	{
		parser.parse(argc, argv);
	}
	catch (argpar::parse_error& e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}

	if (has_help)
	{
		parser.print_help(std::cout);
		return 0;
	}

	std::cout << "Hello from example" << std::endl;
}
