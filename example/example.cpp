#include <argpar/argpar.h>

#include <iostream>
#include <vector>
#include <optional>
#include <memory>

int main(int argc, char * argv[])
{
	bool has_version;
	std::string format;
	std::vector<std::string> libs;
	std::string command;
	std::vector<std::string> commandArgs;
	bool has_verbose;
	bool has_help;

	argpar::parser parser;
	parser.option({"V", "version"}, "Prints out version and exits successfully", &has_version); // optional option
	parser.option({"f", "format"}, "Sets format for the output.") // mandatory option
	      .string_val("FORMAT", &format); // mandatory parameter
	parser.option({"v", "verbose"}, "Enables verbose output.", &has_verbose);
	parser.option({"help"}, "Prints out usage and exits successfully", &has_help);
	parser.option({"x"}, "(mandatory) short-only option with a very long description "
	"that will be split into multiple lines as you can clearly see");
	parser.argument().string_val("command", &command);
	parser.argument_list().string_val("arguments", &commandArgs);

	// other use cases
	bool do_magic;
	int magic_level;
	parser.option({"optional-option-with-optional-parameter"}, "Does something mega useful.", &do_magic)
	      .int_val("MAGIC_LEVEL", &magic_level).between(1, 8).with_default(5); // optional parameter

	struct coords
	{
		float x;
		float y;
	};

	class custom_config : public argpar::cfg_base<custom_config, coords>
	{
	public:
		// constrains the parameter value to certain circular area
		custom_config & from_area(coords center, float radius)
		{
			has_area_restriction_ = true;
			area_center_ = center;
			area_radius_ = radius;

			return *this;
		}

		// value_type typedef is inherited from base_cfg
		value_type parse(std::string const &) const
		{
			coords result = {0, 0};
			// parse values from arg
			// check if satisfies constrains, possibly throw argpar::format_error.
			return result;
		}

	private:
		bool has_area_restriction_ = false;
		coords area_center_ = {};
		float area_radius_ = {};
	};

	coords my_coords;
	// parser.argument().custom_val<custom_config>("myCustomConf", &my_coords)
	      // .from_area({0, 0}, 20).with_default(coords{10, 0});

	try
	{
		parser.parse(argc, argv);
	}
	catch (argpar::parse_error & e)
	{
		parser.print_help(std::cout);
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
