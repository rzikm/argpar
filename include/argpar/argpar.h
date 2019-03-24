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
 */

#ifndef ARGPAR_H
#define ARGPAR_H

#include <string>
#include <vector>
#include <exception>
#include <optional>

namespace argpar
{
/**
 * Exception signalling that a command line argument cannot be parsed to the program
 * representation of the option parameter.
 */
class format_error : public std::exception
{
public:
	format_error();
	/**
	 * \param[in] msg Message giving further explanation of the error.
	 */
	format_error(char const * msg);
};

/**
 * Base class of the exceptions possibly thrown when parsing command line arguments.
 */
class parse_error : public std::exception
{
public:
	/**
	 * Gets the name of the option or argument which caused the error.
	 * \return Returns C style string containing the name of the option which caused the error
	 */
	char const * name() const;
protected:
	parse_error() // do not instantiate
	{
	}
};

/**
 * Exception signaling that an unknown option was encountered.
 */
class bad_option : public parse_error
{
};

/**
 * Exception signaling that an incompatible option parameter or argument was encountered.
 */
class bad_value : public parse_error
{
public:
	const char * value() const;
};

/**
 * Exception signaling that a mandatory option was missing from command line args.
 */
class missing_option : public parse_error
{
};

/**
 * Exception signaling that a mandatory option parameter or positional was missing from command line args.
 */
class missing_value : public parse_error
{
};

/**
 * Class which can be used as a mixin for custom parameter types to provide functionality for
 * default values.
 * \par Expected usage:
 * class my_custom_cfg : public argpar::cfg_base<my_custom_cfg, my_custom_value> { ... }
 * \tparam TConfig Class used to configure the parameter, see argpar::value_config::custom_val.
 * \tparam TValue Type of the value of the parameter, see argpar::value_config::custom_val.
 */
template<typename TConfig, typename TValue>
class cfg_base
{
public:
	using value_type = TValue;
	using container = std::vector<TValue>;
	bool has_default()
	{
		return value_.has_value();
	}

	value_type get_default() const
	{
		return value_.value();
	}

	TConfig & with_default(const value_type & value)
	{
		value_ = value;

		return *(TConfig *)this;
	}
private:
	std::optional<value_type> value_;
};

/**
 * Class for configuring a string value of a plain argument or option parameter.
 */
class string_cfg
{
public:
	using value_type = std::string;
	using container = std::vector<value_type>;

	/**
	 * Constrains the parameter to specified set of values.
	 * \param[in] values Allowed set of values.
	 * \return Returns reference to this object for method chaining.
	 */
	string_cfg & from(std::vector<std::string> const & values);

	/**
	 * Configures the parameter to be optional, using the specified value when none is supplied.
	 * \param[in] value Default value of the parameter.
	 * \return Returns reference to this object for method chaining.
	 */
	string_cfg & with_default(std::string const & value);
};

/**
 * Class for configuring an integer value of a plain argument or option parameter.
 */
class int_cfg
{
public:
	using value_type = int;
	using container = std::vector<value_type>;

	/**
	 * Constrains the parameter to fit between specified two values.
	 * \param[in] min Minimum value of the parameter (inclusive).
	 * \param[in] max Maximum value of the parameter (inclusive).
	 * \return Returns reference to this object for method chaining.
	 */
	int_cfg & between(int min, int max);

	/**
	 * Configures the parameter to be optional, using the specified value when none is supplied.
	 * \param[in] value Default value of the parameter.
	 * \return Returns reference to this object for method chaining.
	 */
	int_cfg & with_default(int value);
};

/**
 * Class for configuring a double value of a plain argument or option parameter.
 */
class double_cfg
{
public:
	using value_type = double;
	using container = std::vector<value_type>;

	/**
	 * Constrains the parameter to fit between specified two values.
	 * \param[in] min Minimum value of the parameter (inclusive).
	 * \param[in] max Maximum value of the parameter (inclusive).
	 * \return Returns reference to this object for method chaining.
	 */
	double_cfg & between(double min, double max);

	/**
	 * Configures the parameter to be optional, using the specified value when none is supplied.
	 * \param[in] value Default value of the parameter.
	 * \return Returns reference to this object for method chaining.
	 */
	double_cfg & with_default(double value);
};

class value_list_config
{
public:
	/**
	 * Configures the list to contain integer arguments.
	 * \param[in]  name Name of the parameter to be displayed in the usage
	 * clause.
	 * \param[out] dest Pointer to the container where the parsed arguments should be stored. The
	 * pointer must be valid during the associated parse() call.
	 * \return Returns object for further configuration of the parameter.
	 */
	int_cfg & int_val(std::string const & name, std::vector<int> * dest);

	/**
	 * Configures the list to contain string arguments.
	 * \param[in]  name Name of the parameter to be displayed in the usage
	 * clause.
	 * \param[out] dest Pointer to the container where the parsed arguments should be stored. The
	 * pointer must be valid during the associated parse() call.
	 * \return Returns object for further configuration of the parameter.
	 */
	string_cfg & string_val(std::string const & name, std::vector<std::string> * dest);

	/**
	 * Configures the list to accept a floating point number arguments.
	 * \param[in]  name Name of the parameter to be displayed in the usage
	 * clause.
	 * \param[out] dest Pointer to the container where the parsed arguments should be stored. The
	 * pointer must be valid during the associated parse() call.
	 * \return Returns object for further configuration of the parameter.
	 */
	double_cfg & double_val(std::string const & name, std::vector<double> * dest);

	/**
	 * Configures the option or argument to accept a parameter of custom type.
	 * \tparam     TConfig  Type used to configure the parameter fulfilling following requirements
	 *    (most of which can be fulfilled by publicly deriving from argpar::cfg_base<TConfig, TValue>):
	 *  - public parameterless constructor
	 *  - define TConfig::value_type to be the target type of the value. The type must be
	 *    movable or copyable.
	 *  - define TConfig::container to be the type used to store the values.
	 *  - public instance method TConfig::value_type TConfig::parse(char const *) const, which
	 *    parses the given string parameter and returns an instance of the target type. This method
	 *    should throw argpar::format_error to indicate that incompatible option parameter was used.
	 * \param[in]  name Name of the parameter to be displayed in the usage
	 * clause.
	 * \param[out] dest Pointer to the memory where the parsed parameter should be stored. The
	 * pointer must be valid during the associated parse() call.
	 * \return Returns a reference to an internally constructed TConfig object for further
	 * configuration.
	 */
	template<typename TConfig>
	TConfig & custom_val(std::string const & name, typename TConfig::container * dest)
	{
		return *new TConfig(); // just pacify the compiler
	}
};

/**
 * Class for configuring the parameter of the command line option. If no method on this class is
 * called, then it is assumed that the option has no parameter.
 */
class value_config
{
public:
	/**
	 * Configures the option or argument to accept an integer parameter.
	 * \param[in]  name Name of the parameter to be displayed in the usage
	 * clause.
	 * \param[out] dest Pointer to the memory where the parsed parameter should be stored. The
	 * pointer must be valid during the associated parse() call.
	 * \return Returns object for further configuration of the parameter.
	 */
	int_cfg & int_val(std::string const & name, int * dest);

	/**
	 * Configures the option or argument to accept a string parameter.
	 * \param[in]  name Name of the parameter to be displayed in the usage
	 * clause.
	 * \param[out] dest Pointer to the memory where the parsed parameter should be stored. The
	 * pointer must be valid during the associated parse() call.
	 * \return Returns object for further configuration of the parameter.
	 */
	string_cfg & string_val(std::string const & name, std::string * dest);

	/**
	 * Configures the option or argument to accept a floating point number parameter.
	 * \param[in]  name Name of the parameter to be displayed in the usage
	 * clause.
	 * \param[out] dest Pointer to the memory where the parsed parameter should be stored. The
	 * pointer must be valid during the associated parse() call.
	 * \return Returns object for further configuration of the parameter.
	 */
	double_cfg & double_val(std::string const & name, double * dest);

	/**
	 * Configures the option or argument to accept a parameter of custom type.
	 * \tparam     TConfig  Type used to configure the parameter fulfilling following requirements
	 *    (most of which can be fulfilled by publicly deriving from argpar::cfg_base<TConfig, TValue>):
	 *  - public parameterless constructor
	 *  - define TConfig::value_type to be the target type of the value. The type must be
	 *    movable or copyable.
	 *  - public instance method TConfig::value_type TConfig::parse(char const *) const, which
	 *    parses the given string parameter and returns an instance of the target type. This method
	 *    should throw argpar::format_error to indicate that incompatible option parameter was used.
	 *    public instance method std::optional<TConfig::value_type> TConfig::default() const, which
	 *    returns a default instance or std::nullopt if no default is to be provided. The default
	 *    will be requested during the argpar::parser::parse call only.
	 *  - public instance method bool TConfig::has_default() const, which returns whether the
	 *    configured parameter/option has default value configured
	 *  - public instance method TConfig::value_type get_default() const, which returns the default if
	 *    the above method returns true.
	 * \param[in]  name Name of the parameter to be displayed in the usage
	 * clause.
	 * \param[out] dest Pointer to the memory where the parsed parameter should be stored. The
	 * pointer must be valid during the associated parse() call.
	 * \return Returns a reference to an internally constructed TConfig object for further
	 * configuration of the parameter.
	 */
	template<typename TConfig>
	TConfig & custom_val(std::string const & name, typename TConfig::value_type * dest)
	{
		return *new TConfig(); // just pacify the compiler
	}
};

/**
 * Main class for parsing command line options.
 */
class parser
{
public:
	/**
	 * Defines a new mandatory option
	 * \param[in] aliases Aliases for the option. One character strings are taken as short options,
	 * Two or more characters are taken as long option.
	 * \param[in] hint    Description of the option to be printed in the usage clause.
	 * \throw std::invalid_argument If no aliases are given, some alias is already used for
	 * another option or some alias contains forbidden characters.
	 * \return Reference to an object which can be used to further configure the option.
	 */
	value_config & option(std::vector<std::string> const & aliases, std::string const & hint);

	/**
	 * Defines a new optional option.
	 * \param[in]  aliases   Aliases for the option.One character strings are taken as short
	 * options, Two or more characters are taken as long option.
	 * \param[in]  hint      Description of the option to be printed in the usage clause.
	 * \param[out] flag_dest  Pointer to a boolean flag to be set if the option was present during
	 * parsing. The pointer must be valid during the associated parse() call.
	 * \throw std::invalid_argument If no aliases are given, some alias is already used for
	 * another option or some alias contains forbidden characters.
	 * \return Reference to an object which can be used to further configure the option.
	 */
	value_config & option(std::vector<std::string> const & aliases, std::string const & hint, bool * flag_dest);

	/**
	 * Defines a new positional argument. Use method chaining to further configure it's type.
	 * \throw std::logic_error if argument_list() method has been called before.
	 * \return Reference to an object which can be used to cofigure the value of the argument.
	 */
	value_config & argument();

	/**
	 * Defines a variable number of positional arguments of the same type. No more arguments can be
	 * specified after this call.
	 * \throw std::logic_error if argument_list() method has already been called before.
	 * \return Reference to an object which can be used to cofigure the value of the argument.
	 */
	value_list_config & argument_list();

	/**
	 * Parses the options from the given command line arguments.
	 * \param[in] argc Number of arguments.
	 * \param[in] argv Array of length argc containing the command-line arguments to be parsed;
	 * \throw std::logic_error       if the configuration is ambiguous (optional positional argument
	 *                               followed by mandatory argument) or some positional argument
	 *                               does not have the type configured.
	 * \throw argpar::parse_error    when parsing arguments failed. Base class of all following
	 *                               exceptions.
	 * \throw argpar::bad_option     when unknown option is encountered.
	 * \throw argpar::bad_value      when incompatible option parameter or argument is encountered.
	 * \throw argpar::missing_option if a mandatory option is not present.
	 * \throw argpar::missing_value  if a mandatory parameter to an option or a mandatory positional
	 *                               argument is not present.
	 */
	void parse(int argc, char ** argv);

	/**
	 * Prints help clause describing usage of all registered options and parameters in a readable format.
	 * \param[out] stream Stream to be written into.
	 */
	void print_help(std::ostream & stream);
};
}
#endif // ARGPAR_H
