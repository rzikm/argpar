/** \file
 *
 * argpar: a simple library for parsing command line arguments and options.
 *
 * The library uses fluent API to declaratively state the command line
 * options. There were few key ideas involved during the design:
 *  - locality: Everything that concerns command line options is located at
 *  	one place in code, no copying of magical string constants or need for
 *  	defining global string constants needed.
 *  - readability: With fluent API the declaration of command line arguments
 *  	reads almost like a sentence. And because the description of the options
 *  	and their parameters are included in the declaration, it serves also as
 *  	in in-code documentation.
 *  
 * Features:
 * 	- Mandatory or optional options with short and/or long names
 * 	- Option parameters:
 * 		- none
 * 		- int
 * 			- any
 * 			- between specified min and max value
 * 		- double
 * 			- any
 * 			- between specified min and max value
 * 		- string
 * 			- any
 * 			- from allowed set of values
 * 	- Ability to set default option parameter, which makes the parameter optional
 * 	- Generating help clause
 *
 * Example:
 *
 *	// storage for the parsed data
 *	bool has_version;
 *	std::string format;
 *	bool has_verbose;
 *	int magic_level;
 *	
 *	// define options
 *	argpar::parser parser;
 *	parser.option({ "V", "version" }, "Prints out version and exits successfully", &has_version); // optional option
 *	parser.option({ "f", "format" }, "Sets format for the output.") // mandatory option
 *		.string_par("FORMAT", &format); // mandatory string parameter
 *	parser.option({ "use-magic" }, "Does something useful.", &do_magic) // optional option
 *		.integer_par("MAGIC_LEVEL", &magic_level).between(0, 8).with_default(0); // optional int parameter with constraints
 *	
 *	try
 *	{
 *	 	// consumes options and their parameters, sets argc to number of plain
 *	 	// arguments and argv to point to the first plain argument.
 *		parser.parse(&argc, &argv);
 *	}
 *	catch (std::exception& e)
 *	{
 *		std::cout << e.what() << std::endl;
 *		return -1;
 *	}
 *	
 *	// process plain args as usual
 *	std::vector<std::string> args(argv, argv + argc);
 *	for(auto && arg : args)
 *	{
 *		//...
 *	}
 */

#ifndef ARGPAR_H
#define ARGPAR_H

#include <string>
#include <vector>

namespace argpar
{
	/**
	 * Class for further configuring of option with string parameter.
	 */
	class string_par_config 
	{
	public:
		/**
		 * Constrains the parameter to specified set of values.
		 * \param[in] values Allowed set of values.
		 * \return Returns reference to this object for method chaining.
		 */
		string_par_config & from(std::vector<std::string> const & values);
		/**
		 * Configures the parameter to be optional, using the specified value when
		 * none is supplied.
		 * \param[in] value Default value of the parameter.
		 * \return Returns reference to this object for method chaining.
		 */
		string_par_config & with_default(std::string const & value);
	};

	/**
	 * Class for further configuring of option with integer parameter.
	 */
	class integer_par_config 
	{
	public:
		/**
		 * Constrains the parameter to fit between specified two values.
		 * \param[in] min Minimum value of the parameter (inclusive).
		 * \param[in] max Maximum value of the parameter (inclusive).
		 * \return Returns reference to this object for method chaining.
		 */
		integer_par_config & between(int min, int max);
		/**
		 * Configures the parameter to be optional, using the specified value when
		 * none is supplied.
		 * \param[in] value Default value of the parameter.
		 * \return Returns reference to this object for method chaining.
		 */
		integer_par_config & with_default(int value);
	};

	/**
	 * Class for further configuring of option with floating point parameter.
	 */
	class double_par_config 
	{
	public:
		/**
		 * Constrains the parameter to fit between specified two values.
		 * \param[in] min Minimum value of the parameter (inclusive).
		 * \param[in] max Maximum value of the parameter (inclusive).
		 * \return Returns reference to this object for method chaining.
		 */
		double_par_config & between(double min, double max);
		/**
		 * Configures the parameter to be optional, using the specified value when
		 * none is supplied.
		 * \param[in] value Default value of the parameter.
		 * \return Returns reference to this object for method chaining.
		 */
		double_par_config & with_default(double value);
	};

	/**
	 * Class for configuring the parameter of the command line option. If no
	 * method on this class is called, then it is assumed that the option has no
	 * parameter.
	 */
	class opt_config 
	{
	public:
		/**
		 * Configures the option to accept an integer parameter.
		 * \param[in]  name     Name of the parameter to be displayed in the usage
		 * clause.
		 * \param[out] par_dest Pointer to the memory where the parsed parameter
		 * should be stored. The pointer must be valid during the associated
		 * parse() call.
		 * \return Returns object for further configuration of the parameter.
		 */
		integer_par_config & integer_par(std::string const & name, int * par_dest);
		/**
		 * Configures the option to accept a string parameter.
		 * \param[in]  name     Name of the parameter to be displayed in the usage
		 * clause.
		 * \param[out] par_dest Pointer to the memory where the parsed parameter
		 * should be stored. The pointer must be valid during the associated
		 * parse() call.
		 * \return Returns object for further configuration of the parameter.
		 */
		string_par_config & string_par(std::string const & name, std::string * par_dest);
		/**
		 * Configures the option to accept a floating point number parameter.
		 * \param[in]  name     Name of the parameter to be displayed in the usage
		 * clause.
		 * \param[out] par_dest Pointer to the memory where the parsed parameter
		 * should be stored. The pointer must be valid during the associated
		 * parse() call.
		 * \return Returns object for further configuration of the parameter.
		 */
		double_par_config & double_par(std::string const & name, double * par_dest);
	};

	/**
	 * Main class for parsing command line options.
	 */
	class arg_parser
	{
	public:
		/**
		 * Defines a new mandatory option
		 * \param[in] aliases Aliases for the option. One character strings are
		 * taken as short options, Two or more characters are taken as long
		 * option.
		 * \param[in] hint    Description of the option to be printed in the usage
		 * clause. 
		 * \return Reference to object which can be used to further configure the
		 * option.
		 */
		opt_config & option(std::vector<std::string> const & aliases, std::string const & hint);
		/**
		 * Defines a new optional option.
		 * \param[in]  aliases   Aliases for the option.One character strings are
		 * taken as short options, Two or more characters are taken as long
		 * option.
		 * \param[in]  hint      Description of the option to be printed in the usage
		 * clause.
		 * \param[out] opt_dest  Pointer to a boolean flag to be set if the option
		 * was present during parsing. The pointer must be valid during the
		 * associated parse() call.
		 * \return Reference to object which can be used to further configure the option.
		 */
		opt_config & option(std::vector<std::string> const & aliases, std::string const & hint, bool * opt_dest);

		/**
		 * Parses the options from the given command line arguments and leaves only
		 * the plain arguments there.
		 * \param[in,out] p_argc Pointer to the number of arguments.
		 * \param[in,out] p_argv Pointer to the array containing the command-line
		 * argument values.
		 */
		void parse(int * p_argc, char *** p_argv);

		/**
		 * Prints help clause describing usage of all registered options.
		 * \param[out] stream Stream to be written into.
		 */
		void print_help(std::ostream & stream);
	};

}
#endif // ARGPAR_H
