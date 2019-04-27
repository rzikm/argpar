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
		// if (positional_arguments_.size() > 0 && positional_arguments_[positional_arguments_.size()]->)
		return *positional_arguments_.emplace_back(std::make_unique<value_config>());
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
		return additional_arguments_.emplace();
	}

	void set_parsed_value(detail::option *& current_option, std::string option_name, std::optional<std::string> value)
	{
		try
		{
			switch (current_option->arg_type())
			{
			case detail::option::arg_type::no_arg:
				if (value.has_value())
					throw argpar::bad_value(option_name, value.value(), helpers::make_str("Option '", option_name, "' does not take any values"));
				current_option = nullptr; // parsing finished 
				break;
			case detail::option::arg_type::optional:
				if (value.has_value())
					current_option->value_cfg().handler_->parse(value.value());
				else
					current_option->value_cfg().handler_->set_default();
				current_option = nullptr; // parsing finished either way
				break;
			case detail::option::arg_type::mandatory:
				if (value.has_value())
				{
					current_option->value_cfg().handler_->parse(value.value());
					current_option = nullptr; // parsing finished 
				}
				break;
			}
		}
		catch (argpar::format_error& e)
		{
			throw argpar::bad_value(option_name, value.value(), e.message());
		}
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
		// TODO: Check validity

		size_t i = 1;
		std::vector<std::string> args(argv, argv + argc);

		detail::option * current_option = nullptr; // currently processed option
		std::string option_name;
		std::optional<std::string> value;
		for (; i < args.size(); ++i)
		{
			value.reset();
			std::string & arg = args[i];

			if (arg == "--") // positional argument delimiter
			{
				++i;
				break;
			}

			if (current_option) // still processing some option
			{
				set_parsed_value(current_option, option_name, arg);
				continue;
			}

			if (arg.size() > 1 && arg[0] == '-')
			{
				if (arg[1] == '-') // long option
				{
					size_t eq_pos = arg.find('=');
					if (eq_pos != std::string::npos)
					{
						// option_name = arg.substr(2, eq_pos - 2);
						value = arg.substr(eq_pos + 1);
					}
					option_name = arg.substr(2, eq_pos - 2);
					// option_name = arg.substr(2);
					current_option = find_option(option_name);
				}
				else // short option
				{
					// set all condensed flags
					size_t flag_pos = 0;
					while (flag_pos < arg.size() - 1)
					{
						current_option = find_option(arg[++flag_pos]);
						if (current_option->arg_type() != detail::option::arg_type::no_arg) break;
						current_option->set_found();
					}
					option_name = std::string(1, arg[flag_pos]);
				}

				current_option->set_found();
				set_parsed_value(current_option, option_name, value);
			}
			else
			{
				// first positional arguments encountered
				break;
			}
		}

		if (current_option)
			throw argpar::missing_value(option_name);

		verify_options();

		// process positional args
		size_t pos = 0;
		bool optional_only = false;
		for (; i < args.size(); ++i, ++pos)
		{
			std::string & arg = args[i];
			if (pos < positional_arguments_.size())
			{
				value_config & config = *positional_arguments_[pos];
				config.handler_->parse(arg);
			}
			else
			{
				if (!additional_arguments_.has_value())
					throw std::logic_error("Too many arguments.");
				additional_arguments_->handler_->parse(arg);
			}
		}
	}

	void print_usage_line(std::ostream & stream)
	{
		stream << "Usage: ";
		if (!options_.empty())
		{
			stream << "[OPTIONS...] ";
		}
		// for (auto && opt : options_)
		// {
			// if (opt->mandatory())
			// {
				// stream << ;
			// }
		// }
		for (auto && arg : positional_arguments_)
		{
			stream << "[" << arg->handler_->name() << "] ";
		}
		if (additional_arguments_.has_value())
		{
			stream << "[" << additional_arguments_.value().handler_->name() << "...] ";
		}
		stream << "\n";
	}

	/**
	 * Prints help clause describing usage of all registered options and parameters in a readable format.
	 * \param[out] stream Stream to be written into.
	 */
	void print_help(std::ostream & stream) {
		print_usage_line(stream);

		if (!options_.empty())
		{
			stream << "Options: \n";
			for (auto && opt : options_)
			{
				stream << "\t";
				bool first = true;
				if (opt->short_name())
				{
					stream << "-" << opt->short_name() << ", ";
				}
				else
				{
					stream << "    ";
				}

				if (!opt->long_name().empty())
				{
					stream << "--" << opt->long_name() << " ";
				}
				if (opt->value_cfg().handler_)
				{
					// adequate set of parentheses
					const char * parens = opt->value_cfg().handler_->has_default()
						? "[]"
						: "<>";
					auto && handler = opt->value_cfg().handler_;
					stream << parens[0] << handler->name();
					if (handler->has_default())
					{
						stream << " = "; handler->print_default(stream);
					}
					stream << parens[1];
				}
				stream << "\n\t\t" << opt->hint();

				stream << "\n\n";
			}
		}
	}

private:
	argpar::value_config & option_unchecked(std::vector<std::string> const & aliases, std::string const & hint, bool * dest)
	{
		std::unique_ptr<detail::option> option = std::make_unique<detail::option>(aliases, hint, dest);
		add_aliases(option.get());
		options_.emplace_back(std::move(option));
		return options_.back()->value_cfg();
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

	std::vector<std::unique_ptr<argpar::detail::option>> options_;
	std::unordered_map<std::string, argpar::detail::option *> long_to_option_;
	std::unordered_map<char, argpar::detail::option *> short_to_option_;
	std::optional<value_list_config> additional_arguments_;
	std::vector<std::unique_ptr<value_config>> positional_arguments_;
};
}
#endif // ARGPAR_H
