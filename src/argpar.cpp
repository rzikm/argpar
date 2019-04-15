#include <argpar/argpar.h>
#include <algorithm>
#include <sstream>

argpar::string_cfg & argpar::string_cfg::from(std::vector<std::string> const & values)
{
	if (values.empty()) throw std::invalid_argument("Values array must be nonempty");
	return *this;
}

argpar::int_cfg & argpar::value_config::int_val(std::string const & name, int * par_dest)
{
	return custom_val<int_cfg>(name, par_dest);
}

argpar::string_cfg & argpar::value_config::string_val(std::string const & name, std::string * par_dest)
{
	return custom_val<string_cfg>(name, par_dest);
}

argpar::double_cfg & argpar::value_config::double_val(std::string const & name, double * par_dest)
{
	return custom_val<double_cfg>(name, par_dest);
}

argpar::int_cfg & argpar::value_list_config::int_val(std::string const & name, std::vector<int> * par_dest)
{
	return custom_val<int_cfg>(name, par_dest);
}

argpar::string_cfg & argpar::value_list_config::string_val(std::string const & name, std::vector<std::string> * par_dest)
{
	return custom_val<string_cfg>(name, par_dest);
}

argpar::double_cfg & argpar::value_list_config::double_val(std::string const & name, std::vector<double> * par_dest)
{
	return custom_val<double_cfg>(name, par_dest);
}

argpar::value_config & argpar::parser::option(std::vector<std::string> const & aliases, std::string const & hint)
{
	std::unique_ptr<argpar::option> option = std::make_unique<argpar::option>(aliases, hint, nullptr);
	add_aliases(aliases, option.get());
	options_.emplace_back(std::move(option));
	return options_.back()->value_config();
}

argpar::value_config & argpar::parser::option(std::vector<std::string> const & aliases, std::string const & hint,
                              bool * opt_dest)
{
	if (!opt_dest) throw std::invalid_argument("Argument opt_dest cannot be nullptr");

	std::unique_ptr<argpar::option> option = std::make_unique<argpar::option>(aliases, hint, opt_dest);
	add_aliases(aliases, option.get());
	options_.emplace_back(std::move(option));
	return options_.back()->value_config();
}

void argpar::parser::add_aliases(std::vector<std::string> const & aliases, argpar::option * option)
{
	// check for duplicate aliases
	if (aliases.empty()) throw std::invalid_argument("Set of aliases cannot be empty");
	for (auto && alias : aliases)
	{
		if (alias.empty()) throw std::invalid_argument("Alias cannot be zero length");
		bool success;
		if (alias.size() == 1)
		{
			auto [_, inserted] = short_to_option_.try_emplace(alias[0], option);
			success = inserted;
		}
		else
		{
			auto [_, inserted] = long_to_option_.try_emplace(alias, option);
			success = inserted;
		}
		if (!success) throw std::invalid_argument(make_str("Duplicate alias definition: ", alias, "."));
	}
}

argpar::value_config & argpar::parser::argument()
{
	if (additional_arguments_.has_value()) throw std::logic_error("Positional arguments cannot be defined after argument_list() was called");
	// if (positional_arguments_.size() > 0 && positional_arguments_[positional_arguments_.size()]->)
	return *positional_arguments_.emplace_back(std::make_unique<value_config>());
}

argpar::value_list_config & argpar::parser::argument_list()
{
	if (additional_arguments_.has_value()) throw std::logic_error("Function argument_list() was already called");
	return additional_arguments_.emplace();
}

void argpar::parser::set_parsed_value(argpar::option *& current_option, std::string option_name, std::optional<std::string> value)
{
	switch (current_option->arg_type())
	{
	case option::arg_type::no_arg:
		if (value.has_value())
			throw argpar::bad_value(option_name, value.value(), make_str("Option '", option_name, "' does not take any values"));
		break;
	case option::arg_type::optional:
		if (value.has_value())
			current_option->value_config().handler_->parse(value.value());
		else
			current_option->value_config().handler_->set_default();
		current_option = nullptr; // parsing finished either way
		break;
	case option::arg_type::mandatory:
		if (value.has_value())
		{
			current_option->value_config().handler_->parse(value.value());
			current_option = nullptr; // parsing finished 
		}
		break;
	}
}

void argpar::parser::parse(int argc, char ** argv)
{
	if (argc < 1) throw std::invalid_argument("Parameter argc cannot be less than 1.");
	if (!argv) throw std::invalid_argument("Parameter argv cannot be nullptr.");
	// TODO: Check validity

	size_t i = 1;
	std::vector<std::string> args(argv, argv + argc);

	argpar::option * current_option = nullptr; // currently processed option
	std::string option_name;
	std::optional<std::string> value;
	for(; i < args.size(); ++i)
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
			current_option->value_config().handler_->parse(arg);
			current_option = nullptr;
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
				current_option->set_found();
			}
			else // short option
			{
				// set all condensed flags
				size_t flag_pos = 0;
				while (flag_pos < arg.size() - 1)
				{
					current_option = find_option(arg[++flag_pos]);
					if (current_option->arg_type() != option::arg_type::no_arg) break;

					current_option->set_found();
				}
				option_name = std::to_string(arg[flag_pos]);
			}

			try
			{
				set_parsed_value(current_option, option_name, value);
			}
			catch (argpar::format_error& e)
			{
				if (e.message().empty())
					throw argpar::bad_value(option_name, value.value());
				throw argpar::bad_value(option_name, value.value(), e.message());
			}
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

void argpar::parser::print_help(std::ostream & stream)
{
}
