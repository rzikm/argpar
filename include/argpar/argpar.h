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
#include <unordered_map>
#include <exception>
#include <optional>
#include <sstream>
#include <algorithm>
#include <memory>

namespace argpar
{
	// helpers
template <typename THead, typename ... TTail>
void print_to(std::ostream & stream, THead && head, TTail && ... values)
{
	stream << head;
	if constexpr(sizeof...(values) > 0)
	{
		print_to(stream, values...);
	}
}

template <typename ... T>
std::string make_str(T && ... values)
{
	std::stringstream ss;
	print_to(ss, values...);
	return ss.str();
}

// forward declarations
	class parser;
	class option;


class _no_copy_move
{
protected:
	_no_copy_move()
	{ // allow default constructor
	}
private:
	_no_copy_move(const _no_copy_move &) = delete;
	_no_copy_move(_no_copy_move &&) = delete;
	_no_copy_move & operator=(const _no_copy_move &) = delete;
	_no_copy_move & operator=(_no_copy_move &&) = delete;
};

class argpar_exception: public std::exception
{
public:
	argpar_exception(std::string const & message)
		: message_(message)
	{
	}

	char const * what() const noexcept override
	{
		return message_.c_str();
	}

	std::string const & message() const noexcept
	{
		return message_;
	}
private:
	std::string message_;
};

/**
 * Exception signalling that a command line argument cannot be parsed to the program
 * representation of the option parameter.
 */
class format_error : public argpar_exception
{
public:
		/**
	 * \param[in] msg Message giving further explanation of the error.
	 */
	format_error(std::string const & message)
		: argpar_exception(message)
	{
	}
};

/**
 * Base class of the exceptions possibly thrown when parsing command line arguments.
 */
class parse_error : public argpar_exception
{
public:
	/**
	 * Gets the name of the option or argument which caused the error.
	 * \return Returns C style string containing the name of the option which caused the error
	 */
	char const * name() const
	{
		return name_.c_str();
	}
protected:
	parse_error(std::string const & name, std::string const & message) 
		: name_(name)
		, argpar_exception(message)
	{
	}

private:
	std::string name_;
};

/**
 * Exception signaling that an unknown option was encountered.
 */
class bad_option : public parse_error
{
public:
	bad_option(std::string const & name)
		: argpar::parse_error(name, "")
	{
	}
};

/**
 * Exception signaling that an incompatible option parameter or argument was encountered.
 */
class bad_value : public parse_error
{
public:
	bad_value(std::string const & name, std::string const & value)
		: parse_error(name, make_str("Invalid value for option '", name, "'."))
	{
	}
	bad_value(std::string const & name, std::string const & value, std::string const & message)
		: parse_error(name, message)
	{
	}
	char const * value() const
	{
		return value_.c_str();
	}
private:
	std::string value_;
};

/**
 * Exception signaling that a mandatory option was missing from command line args.
 */
class missing_option : public parse_error
{
public:
	missing_option(std::string const & name)
		: parse_error(name, make_str("Mandatory option missing: '", name, "'."))
	{
	}
};

/**
 * Exception signaling that a mandatory option parameter or positional was missing from command line args.
 */
class missing_value : public parse_error
{
public:
	missing_value(std::string const & name)
		: parse_error(name, make_str("Missing value for option '", name, "'."))
	{
	}
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
class cfg_base : private _no_copy_move
{
	cfg_base(cfg_base const &) = delete;
protected:
	cfg_base()
	{
	}

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

	/**
	 * Configures the parameter to be optional, using the specified value when none is supplied.
	 * \param[in] value Default value of the parameter.
	 * \return Returns reference to this object for method chaining.
	 */
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
class string_cfg : public cfg_base<string_cfg, std::string>
{
public:
	/**
	 * Constrains the parameter to specified set of values.
	 * \param[in] values Allowed set of values.
	 * \return Returns reference to this object for method chaining.
	 */
	string_cfg & from(std::vector<std::string> const & values);

	std::string parse(std::string const & value)
	{
		if (!values_.empty() && std::find(values_.begin(), values_.end(), value) != values_.end())
		{
			throw argpar::format_error(make_str("Value '", value, "' is not allowed."));
		}

		return value;
	}

private:
	//! List of allowed values. If empty, any value is allowed
	std::vector<std::string> values_;
};

template<typename TConfig, typename TValue>
class cfg_integral_base : public cfg_base<TConfig, TValue>
{
public:
	/**
	 * Constrains the parameter to fit between specified two values.
	 * \param[in] min Minimum value of the parameter (inclusive).
	 * \param[in] max Maximum value of the parameter (inclusive).
	 * \return Returns reference to this object for method chaining.
	 */
	TConfig & between(TValue min, TValue max)
	{
		min_value_ = min;
		max_value_ = max;
		return *(TConfig *)this;
	}

	TValue parse(std::string const & value)
	{
		TValue val;
		bool valid = true;

		std::stringstream ss(value);
		ss >> val;

		if (!ss || ss.tellg() != value.size())
			throw argpar::format_error(make_str("Value '", value, "' does not represent a valid number."));

		if (!valid || val > max_value_ || val < min_value_)
			throw argpar::format_error(make_str("Value '", value, "' is out of bounds."));

		return val;
	}

private:
	TValue min_value_ = std::numeric_limits<TValue>::min();
	TValue max_value_ = std::numeric_limits<TValue>::max();
};

/**
 * Class for configuring an integer value of a plain argument or option parameter.
 */
class int_cfg : public cfg_integral_base<int_cfg, int>
{
};

/**
 * Class for configuring a double value of a plain argument or option parameter.
 */
class double_cfg : public cfg_integral_base<double_cfg, double>
{
};

class value_handler
{
public:
	virtual void parse(std::string const & value) = 0;
	virtual bool has_default() = 0;
	virtual void set_default() = 0;

	virtual ~value_handler()
	{
	}
};

template<typename TConfig>
class value_handler_base : public value_handler
{
public:
	virtual bool has_default() override
	{
		return config_.has_default();
	}

	TConfig & get_config()
	{
		return config_;
	}
private:
	TConfig config_;
};

template<typename TConfig>
class single_value_handler : public value_handler_base<TConfig>
{
	using value_type = typename TConfig::value_type;
	using base = value_handler_base<TConfig>;
public:
	single_value_handler(value_type * dest)
		: dest_(dest)
	{
	}

	void parse(std::string const & value) override
	{
		*dest_ = base::get_config().parse(value);
	}

	virtual void set_default() override
	{
		*dest_ = base::get_config().get_default();
	}

	value_type * dest_;
};

template<typename TConfig>
class multi_value_handler : public value_handler_base<TConfig>
{
	using container = typename TConfig::container;
	using base = value_handler_base<TConfig>;
public:
	multi_value_handler(container * dest)
		: dest_(dest)
	{
	}
	void parse(std::string const & value) override
	{
		dest_->emplace_back(base::get_config().parse(value));
	}

	virtual void set_default() override
	{ // do nothing
	}

	container * dest_;
};

class value_list_config : private _no_copy_move
{
	friend class argpar::parser;
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
	 *  - public instance method TConfig::value_type TConfig::parse(std::string const &) const, which
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
		if (name.empty()) throw std::invalid_argument("Name cannot be empty");
		if (dest == nullptr) throw std::invalid_argument("Destination pointer cannot be nullptr");
		if (this->handler_) throw std::logic_error("A value has already been configured");
		std::unique_ptr<multi_value_handler<TConfig>> handler = std::make_unique<multi_value_handler<TConfig>>(dest);
		TConfig * ret = &handler->get_config();
		this->handler_ = std::move(handler);
		return *ret;
	}

private:
	//! Pointer to the value handler, nullptr if no value configured (Option is only a flag).
	std::unique_ptr<value_handler> handler_;
};

/**
 * Class for configuring the parameter of the command line option. If no method on this class is
 * called, then it is assumed that the option has no parameter.
 */
class value_config : private _no_copy_move
{
	friend class argpar::parser;
	friend class argpar::option;
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
	 *  - public instance method TConfig::value_type TConfig::parse(std::string const &) const, which
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
		if (name.empty()) throw std::invalid_argument("Name cannot be empty");
		if (dest == nullptr) throw std::invalid_argument("Destination pointer cannot be nullptr");
		if (this->handler_) throw std::logic_error("A value has already been configured");
		std::unique_ptr<single_value_handler<TConfig>> handler = std::make_unique<single_value_handler<TConfig>>(dest);
		TConfig * ret = &handler->get_config();
		this->handler_ = std::move(handler);
		return *ret;
	}
private:
	//! Pointer to the value handler, nullptr if no value configured (Option is only a flag).
	std::unique_ptr<value_handler> handler_;
};

class option
{
public:
	enum class arg_type
	{
		no_arg,
		optional,
		mandatory
	};

	option(std::vector<std::string> const & aliases, std::string const & hint, bool * flag_dest)
		: aliases_(aliases)
		, hint_(hint)
		, flag_dest_(flag_dest)
		, found_(false)
	{
	}

	value_config & value_cfg()
	{
		return value_config_;
	}

	bool mandatory() const
	{
		return flag_dest_ == nullptr;
	}

	arg_type arg_type() const
	{
		if (!value_config_.handler_) return arg_type::no_arg;
		return value_config_.handler_->has_default()
			? arg_type::optional
			: arg_type::mandatory;
	}

	std::vector<std::string> const & aliases()
	{
		return aliases_;
	}

	void set_found()
	{
		found_ = true;
		if (flag_dest_)
			*flag_dest_ = true;
	}

	bool found()
	{
		return found_;
	}
private:
	std::vector<std::string> aliases_;
	std::string hint_;
	bool * flag_dest_;
	value_config value_config_;
	bool found_;
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
	void set_parsed_value(argpar::option *& current_option, std::string option_name, std::optional<std::string> value);

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

private:
	argpar::option * find_option(std::string const & name)
	{
		auto it = long_to_option_.find(name);
		if (it == long_to_option_.end())
		{
			throw argpar::bad_option(name);
		}

		return it->second;
	}

	argpar::option * find_option(char name)
	{
		auto it = short_to_option_.find(name);
		if (it == short_to_option_.end())
		{
			throw argpar::bad_option(std::to_string(name));
		}

		return it->second;
	}

	void verify_options()
	{
		for (auto && option : options_)
		{
			if (option->mandatory() && !option->found())
				throw argpar::missing_option(option->aliases()[0]);
		}
	}

	void add_aliases(std::vector<std::string> const & aliases, argpar::option * option);

	std::vector<std::unique_ptr<argpar::option>> options_;
	std::unordered_map<std::string, argpar::option *> long_to_option_;
	std::unordered_map<char, argpar::option *> short_to_option_;
	std::optional<value_list_config> additional_arguments_;
	std::vector<std::unique_ptr<value_config>> positional_arguments_;
};
}
#endif // ARGPAR_H
