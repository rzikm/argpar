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


	using arg_iterator_t = std::vector<std::string>::const_iterator;
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
		formatter_.set_cmd(argv[0]);

		std::vector<std::string> args(argv, argv + argc);

		arg_iterator_t arg_it = args.begin();
		arg_iterator_t end = args.end();
		while (arg_it != end) {
			auto arg = *arg_it;
			if (arg == "--") { // positional argument delimiter
				verify_options();
				arg_it = parse_positional_arguments(arg_it, end);
			} else
			if (arg.size() > 1 && arg[0] == '-'){
				arg_it = parse_option(arg_it, end);
			}
			arg_it++;
		}
	}

	arg_iterator_t parse_option(arg_iterator_t arg_it, const arg_iterator_t& end) {
		std::string name = *arg_it;
		std::optional<std::string> value;
		if (name[1] == '-') { //long name
			name = name.erase(0, 2);
			size_t eq_pos = name.find('=');
			if (eq_pos != std::string::npos) {
				value = name.substr(eq_pos + 1);
				name = name.erase(eq_pos);
			}
		} else { //short name
			name = name.erase(0, 1);
			if (name.size() > 1) {
				value = name.substr(1);
				name = name.erase(1);
			}
		}

		detail::option * parsed_option = find_option(name);
		parsed_option->set_found();
		auto type = parsed_option->arg_type();
		switch (type) {
		case parsed_option->arg_type::no_arg:
			set_parsed_value(parsed_option, name, value);
			break;
		case parsed_option->arg_type::optional:
			set_parsed_value(parsed_option, name, value);
			break;
		case parsed_option->arg_type::mandatory:
			if (!value.has_value()){
				arg_it++;
				if (arg_it == end) throw argpar::missing_value(name);
				value = *arg_it;
			}
			set_parsed_value(parsed_option, name, value);
			break;
		}
		return arg_it;

}

	arg_iterator_t parse_positional_arguments(arg_iterator_t arg_it, const arg_iterator_t &end) {
		size_t pos = 0;
		while (arg_it != end) {
			const std::string & arg = *arg_it;
			if (pos < positional_arguments_.size()) {
				detail::positional_argument & config = *positional_arguments_[pos];
				config.handler()->parse(arg);
			} else {
				if (!additional_arguments_.has_value())
					throw std::logic_error("Too many arguments.");
				else
					additional_arguments_->handler()->parse(arg);
			}
			pos++;
			arg_it++;
		}
		return arg_it;
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
	void parseOld(int argc, char ** argv)
	{
		if (argc < 1) throw std::invalid_argument("Parameter argc cannot be less than 1.");
		if (!argv) throw std::invalid_argument("Parameter argv cannot be nullptr.");
		formatter_.set_cmd(argv[0]);
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
				detail::positional_argument & config = *positional_arguments_[pos];
				config.handler()->parse(arg);
			}
			else
			{
				if (!additional_arguments_.has_value())
					throw std::logic_error("Too many arguments.");
				additional_arguments_->handler()->parse(arg);
			}
		}
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
					current_option->handler()->parse(value.value());
				else
					current_option->handler()->set_default();
				current_option = nullptr; // parsing finished either way
				break;
			case detail::option::arg_type::mandatory:
				if (value.has_value())
				{
					current_option->handler()->parse(value.value());
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
