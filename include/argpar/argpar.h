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
#include <deque>
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

/*/**
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
	value_config & option(std::vector<std::string> const & aliases, std::string const & hint)
	{
		return option_unchecked(aliases, hint, nullptr);
	}

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
	value_config & option(std::vector<std::string> const & aliases, std::string const & hint, bool * flag_dest)
	{
		if (!flag_dest) throw std::invalid_argument("Argument opt_dest cannot be nullptr");
		return option_unchecked(aliases, hint, flag_dest);
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
	void parse(int argc, char ** argv)
	{
		if (argc < 1) throw std::invalid_argument("Parameter argc cannot be less than 1.");
		if (!argv) throw std::invalid_argument("Parameter argv cannot be nullptr.");
		formatter_.set_cmd(argv[0]);
		// TODO: Check validity

		std::deque<std::string> args(argv + 1, argv + argc);
		parse_options(args);
		parse_arguments(args);
	}

	/**
	 * Prints help clause describing usage of all registered options and parameters in a readable format.
	 * \param[out] stream Stream to be written into.
	 */
	void print_help(std::ostream & stream) 
	{
		formatter_.print_help(stream);
	}

private:
	void parse_options(std::deque<std::string> & args)
	{
		detail::option * current_option = nullptr; // currently processed option
		std::string option_name; // name of currently processed option, as given in args
		while (!args.empty())
		{
			std::string & arg = args.front();

			if (arg == "--") // positional argument delimiter
			{
				args.pop_front();
				break;
			}

			if (current_option) // still processing some option
			{
				parse_and_set_value(current_option, option_name, arg);
				current_option = nullptr;
				args.pop_front();
				continue;
			}

			if (arg.size() > 1 && arg[0] == '-')
			{
				std::tie(option_name, current_option) = parse_option(arg);
				args.pop_front();
			}
			else
			{
				// first positional arguments encountered
				break;
			}
		}

		if (current_option) // option with mandatory param is missing an argument
			throw argpar::missing_value(option_name, true);

		verify_options();
	}

	std::tuple<std::string, detail::option *> parse_option(std::string const & arg)
	{
		std::optional<std::string> value; // value (if any) of given option
		std::string name; // name of currently processed option, as given in args
		detail::option * option;

		if (arg[1] == '-') // long option
		{
			size_t eq_pos = arg.find('=');
			if (eq_pos != std::string::npos)
			{
				// option_name = arg.substr(2, eq_pos - 2);
				value = arg.substr(eq_pos + 1);
			}
			name = arg.substr(2, eq_pos - 2);
			// option_name = arg.substr(2);
			option = find_option(name);
		}
		else // short option
		{
			// set all condensed flags
			size_t flag_pos = 0;
			while (flag_pos < arg.size() - 1)
			{
				option = find_option(arg[++flag_pos]);
				if (option->arg_type() != detail::option::arg_type::no_arg) break;
				option->set_found();
			}
			name = std::string(1, arg[flag_pos]);
			if (flag_pos != arg.size() - 1)
				value = arg.substr(flag_pos + 1);
		}

		option->set_found();
		if (parse_and_set_value(option, name, value))
			option = nullptr;
		return { name, option };
	}

	void parse_arguments(std::deque<std::string> & args)
	{
		// process positional args
		for (auto && arg : positional_arguments_)
		{
			if (args.empty())
			{
				if (arg->handler()->has_default())
					throw missing_value(arg->handler()->name(), false);
					break;
			}
			arg->handler()->parse(args.front());
			args.pop_front();
		}

		if (!args.empty() && !additional_arguments_.has_value())
					throw std::logic_error("Too many arguments.");

		for (auto && val : args)
		{
			additional_arguments_->handler()->parse(val);
		}
	}

	bool parse_and_set_value(detail::option * option, std::string name, std::optional<std::string> value)
	{
		try
		{
			bool finished = true;

			switch (option->arg_type())
			{
			case detail::option::arg_type::no_arg:
				if (value.has_value())
					throw argpar::bad_value(name, value.value(), helpers::make_str("Option '", name, "' does not take any values"));
				return true; // parsing finished 
			case detail::option::arg_type::optional:
				if (value.has_value())
					option->handler()->parse(value.value());
				else
					option->handler()->set_default();
				break;
			case detail::option::arg_type::mandatory:
				if (value.has_value())
					option->handler()->parse(value.value());
				else
					finished = false;
				break;
			}

			return finished;
		}
		catch (argpar::format_error& e)
		{
			throw argpar::bad_value(name, value.value(), e.message());
		}
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
