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

#include "option.h"
#include "helpers.h"
#include "value_handler.h"
#include "exceptions.h"
#include "positional_argument.h"
#include "formatter.h"

namespace argpar
{
	using arg_iterator_t = std::vector<std::string>::const_iterator;

/*/**
 * Main class for parsing command line options.
 */
class parser
{
public:
	/**
	 * Defines a new mandatory option
	 * \param[in] aliases     Aliases for the option. One character strings are taken as short options,
	 * Two or more characters are taken as long option.
	 * \param[in] description Description of the option to be printed in the usage clause.
	 * \throw std::invalid_argument If no aliases are given, some alias is already used for
	 * another option or some alias contains forbidden characters.
	 * \return Reference to an object which can be used to further configure the option.
	 */
	value_config & option(std::vector<std::string> const & aliases, std::string const & description)
	{
		return option_unchecked(aliases, description, nullptr);
	}

	/**
	 * Defines a new optional option.
	 * \param[in]  aliases     Aliases for the option.One character strings are taken as short
	 * options, Two or more characters are taken as long option.
	 * \param[out] flag_dest   Pointer to a boolean flag to be set if the option was present during
	 * \param[in]  description Description of the option to be printed in the usage clause.
	 * parsing. The pointer must be valid during the associated parse() call.
	 * \throw std::invalid_argument If no aliases are given, some alias is already used for
	 * another option or some alias contains forbidden characters.
	 * \return Reference to an object which can be used to further configure the option.
	 */
	value_config & option(std::vector<std::string> const & aliases, bool * flag_dest, std::string const & description)
	{
		if (!flag_dest) throw std::invalid_argument("Argument opt_dest cannot be nullptr");
		return option_unchecked(aliases, description, flag_dest);
	}

	/**
	 * Defines a new positional argument. Use method chaining to further configure it's type.
	 * \throw std::logic_error if argument_list() method has been called before.
	 * \return Reference to an object which can be used to cofigure the value of the argument.
	 */
	value_config & argument()
	{
		if (additional_arguments_.has_value()) throw std::logic_error("Positional arguments cannot be defined after argument_list() was called");
		detail::positional_argument * arg = positional_arguments_.emplace_back(std::make_unique<detail::positional_argument>()).get();
		formatter_.add_argument(arg);
		return *arg;
	}

	/**
	 * Defines a variable number of positional arguments of the same type. No more arguments can be
	 * specified after this call.
	 * \throw std::logic_error if argument_list() method has already been called before.
	 * \return Reference to an object which can be used to cofigure the value of the argument.
	 */
	value_list_config & argument_list()
	{
		if (additional_arguments_.has_value()) throw std::logic_error("Function argument_list() was already called");
		additional_arguments_.emplace();
		formatter_.add_additional_arguments(&*additional_arguments_);
		return *additional_arguments_;
	}

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
	void parse(int argc, char ** argv) {
		if (argc < 1)
			throw std::invalid_argument("Parameter argc cannot be less than 1.");
		if (!argv)
			throw std::invalid_argument("Parameter argv cannot be nullptr.");
		argument_definition_sanity_check();
		reset_options();

		std::vector<std::string> args(argv, argv + argc);

		arg_iterator_t arg_it = args.begin();
		arg_iterator_t end = args.end();
		formatter_.set_cmd(*arg_it++); // SNEAKY: use program name

		arg_it = parse_options(arg_it, end);
		arg_it = parse_positional_arguments(arg_it, end);
	}

	/**
	 * Prints help clause describing usage of all registered options and parameters in a readable format.
	 * \param[out] stream Stream to be written into.
	 */
	void print_help(std::ostream & stream) 
	{
		formatter_.print_help(stream);
	}

	/**
	 * Prints one-line description of the program arguments.
	 * \param[out] stream Stream to be written into.
	 */
	void print_usage(std::ostream & stream) 
	{
		formatter_.print_usage_line(stream);
	}

private:
	void reset_options()
	{
		for (auto && option : options_)
		{
			option->set_found(false);
		}
	}

	std::tuple<detail::option *, std::string, std::optional<std::string>> get_option(arg_iterator_t it)
	{
		std::string name = *it;
		std::optional<std::string> value;
		detail::option * option = nullptr;

		if (name[1] == '-') //long name
		{
			name = name.erase(0, 2);
			size_t eq_pos = name.find('=');
			if (eq_pos != std::string::npos) {
				value = name.substr(eq_pos + 1);
				name = name.erase(eq_pos);
			}
			option = find_option(name);
			option->set_found(true);
		}
		else //short name
		{
			// set all condensed flags
			do
			{
				name = name.erase(0, 1);
				option = find_option(name[0]);
				option->set_found(true);
			} while (name.size() > 1 && option->arg_type() == detail::option::arg_type::no_arg);

			if (name.size() > 1)
			{
				value = name.substr(1);
				name = name.erase(1);
			}
		}

		return { option, name, value };
	}

	arg_iterator_t parse_option(arg_iterator_t arg_it, const arg_iterator_t& end) 
	{
		auto[parsed_option, name, value] = get_option(arg_it);

		// set the value, if appropriate
		auto type = parsed_option->arg_type();
		if (type == detail::option::arg_type::no_arg && value.has_value())
		{
			// no value allowed
			throw argpar::bad_value(name, value.value(), helpers::make_str("Option '", name, "' does not take any values"));
		}

		if (type == detail::option::arg_type::mandatory && !value.has_value())
		{
			// value must be in next token
			arg_it++;
			if (arg_it == end) throw argpar::missing_value(name, true);
			value = *arg_it;
		}

		if (value.has_value())
		{
			try
			{
				parsed_option->handler()->parse(value.value());
			}
			catch (argpar::format_error& e)
			{
				throw argpar::bad_value(name, value.value(), e.message());
			}
		}
		else if (type != detail::option::arg_type::no_arg)
		{
			parsed_option->handler()->set_default();
		}

		arg_it++;
		return arg_it;
	}

	arg_iterator_t parse_options(arg_iterator_t arg_it, arg_iterator_t const & end)
	{
		while(arg_it != end)
		{
			if (arg_it->size() < 1 || (*arg_it)[0] != '-')
			{
				break;
			}
			if (*arg_it == "--")
			{
				arg_it++;
				break;
			}

			arg_it = parse_option(arg_it, end);
		}

		verify_options();
		return arg_it;
	}

	arg_iterator_t parse_positional_arguments(arg_iterator_t arg_it, arg_iterator_t const & end) 
	{
		size_t pos = 0;
		while (arg_it != end)
		{
			const std::string & arg = *arg_it;
			if (pos < positional_arguments_.size())
			{
				detail::positional_argument & config = *positional_arguments_[pos];
				config.handler()->parse(arg);
				config.set_found(true);
			}
			else
			{
				if (!additional_arguments_.has_value())
					throw argpar::argpar_exception("Too many arguments.");
				else
					additional_arguments_->handler()->parse(arg);
			}
			pos++;
			arg_it++;
		}

		verify_arguments();
		return arg_it;
	}

	argpar::value_config & option_unchecked(std::vector<std::string> const & aliases, std::string const & hint, bool * dest)
	{
		std::unique_ptr<detail::option> option = std::make_unique<detail::option>(aliases, hint, dest);
		add_aliases(option.get());
		options_.emplace_back(std::move(option));
		formatter_.add_option(options_.back().get());
		return *options_.back();
	}

	detail::option * find_option(std::string const & name)
	{
		auto it = long_to_option_.find(name);
		if (it == long_to_option_.end())
		{
			throw argpar::bad_option(name);
		}

		return it->second;
	}

	detail::option * find_option(char name)
	{
		auto it = short_to_option_.find(name);
		if (it == short_to_option_.end())
		{
			throw argpar::bad_option(std::string(1, name));
		}

		return it->second;
	}

	void verify_options()
	{
		for (auto && option : options_)
		{
			if (option->mandatory() && !option->found())
				throw argpar::missing_option(option->short_name() ? helpers::make_str(option->short_name()) : option->long_name());
		}
	}

	void verify_arguments()
	{
		for (auto && arg : positional_arguments_)
		{
			if (arg->mandatory() && !arg->found())
				throw argpar::missing_value(arg->handler()->name(), false);
			if (!arg->found())
				arg->handler()->set_default();
		}
	}

	void argument_definition_sanity_check()
	{
		bool optional_only = false;
		for (auto && arg : positional_arguments_)
		{
			if (!arg->handler())
				throw std::logic_error("No value configured for a positional argument.");
			if (arg->mandatory() && optional_only)
				throw std::logic_error("Mandatory arguments cannot follow optional ones.");
			optional_only = !arg->mandatory();
		}
		if (additional_arguments_.has_value() &&
			!additional_arguments_.value().handler())
		{
			throw std::logic_error("No value configured for a argument list.");
		}
	}

	void add_aliases(argpar::detail::option * opt)
	{
		// check for duplicate aliases
		if (opt->short_name())
		{
			auto[_, inserted] = short_to_option_.try_emplace(opt->short_name(), opt);
			if (!inserted) throw std::invalid_argument(helpers::make_str("Duplicate alias definition: ", opt->short_name(), "."));
		}
		if (!opt->long_name().empty())
		{
			auto[_, inserted] = long_to_option_.try_emplace(opt->long_name(), opt);
			if (!inserted) throw std::invalid_argument(helpers::make_str("Duplicate alias definition: ", opt->long_name(), "."));
		}
	}

	detail::formatter formatter_;
	std::vector<std::unique_ptr<argpar::detail::option>> options_;
	std::unordered_map<std::string, argpar::detail::option *> long_to_option_;
	std::unordered_map<char, argpar::detail::option *> short_to_option_;
	std::optional<detail::positional_argument_list> additional_arguments_;
	std::vector<std::unique_ptr<detail::positional_argument>> positional_arguments_;
};

}
#endif // ARGPAR_H
