#ifndef ARGPAR_FORMATTER_H
#define ARGPAR_FORMATTER_H

#include <iterator>
#include <string_view>
#include "option.h"
#include "iostream"
#include "vector"

namespace argpar::detail
{

// a bit of template/macro magic to make io manipulator declarations simpler
namespace manip 
{ 
template<typename TArg>
struct stream_manip
{
	using func_type = void (*)(std::ostream &, TArg);
	stream_manip(TArg arg, func_type f)
		: arg(arg)
		, func(f)
	{
	}

	func_type func;
	TArg arg;
};

template<typename T>
std::ostream & operator <<(std::ostream & stream, stream_manip<T> manip)
{
	manip.func(stream, manip.arg);
	return stream;
}

template <class F> struct arg_type;
template <class R, class T> struct arg_type<R(*)(std::ostream &, T)> 
{
  typedef T type;
};

#define DEF_MANIP(name, arg_decl)\
	namespace manip { /* forward decl in nested namespace */\
	inline void name##_manip_f(std::ostream &, arg_decl);\
	using name##_arg_t = typename manip::arg_type<decltype(&manip::name##_manip_f)>::type;\
	using name##_manip = manip::stream_manip<name##_arg_t>;\
	}\
	inline manip::name##_manip name(arg_decl);\
	inline manip::name##_manip name(manip::name##_arg_t a)\
	{ /* return wraped in stream_manip<T> instance */\
		return manip::name##_manip(a, manip::name##_manip_f);\
	} /* follows actual manip function definition */\
	inline void manip::name##_manip_f(std::ostream & stream, arg_decl)
} 

DEF_MANIP(option_name, option const * opt)
{
	if (opt->short_name())
	{
		stream << '-' << opt->short_name();
	}
	else
	{
		stream << "--" << opt->long_name();
	}
}

DEF_MANIP(long_opt, option const * opt)
{
	stream << "--" << opt->long_name();
}

DEF_MANIP(short_opt, option const * opt)
{
	stream << "-" << opt->short_name();
}

DEF_MANIP(value_placeholder, value_handler const * handler)
{
	// use adequate set of parentheses
	const char * parens = handler->has_default() ? "[]" : "<>";
	stream << parens[0] << handler->name();
	stream << parens[1];
}

DEF_MANIP(option_placeholder, option const * opt)
{
	// use adequate set of parentheses
	const char * parens = opt->mandatory() ? "<>" : "[]";
	stream << parens[0] << option_name(opt);
	if (opt->handler())
		stream << ' ' << value_placeholder(opt->handler());
	stream << parens[1];
}

DEF_MANIP(alias_list, option const * opt)
{
	if (opt->short_name())
	{
		stream << short_opt(opt);
		if (!opt->long_name().empty())
		{
			stream << ", ";
		}
	}
	else
	{
		stream << "    ";
	}
	if (!opt->long_name().empty())
	{
		stream << long_opt(opt);
	}
}

class formatter
{
	const size_t line_width = 80;
	const size_t alias_indent = 2;
	const size_t desc_indent = 8;
public:
	formatter()
		: additional_arguments_(nullptr)
	{
	}

	void print_help(std::ostream & stream) const
	{
		print_usage_line(stream);

		if (!options_.empty())
			print_options(stream);

		stream << std::flush;
	}

	void print_usage_line(std::ostream & stream) const
	{
		stream << "Usage: " << (cmd_.empty() ? "cmd" : cmd_);
		if (!options_.empty())
		{
			stream << " [OPTIONS...]";
		}

		// list mandatory options
		for (auto && opt : options_)
		{
			if (opt->mandatory())
			{
				stream << ' ' << option_name(opt);
				if (opt->handler())
				{
					stream << ' ' << value_placeholder(opt->handler());
				}
			}
		}

		list_arguments(stream);
		stream << std::endl;
	}

	void add_option(option * opt)
	{
		options_.push_back(opt);
	}

	void add_argument(positional_argument * arg)
	{
		positional_arguments_.push_back(arg);
	}

	void add_additional_arguments(positional_argument_list * args)
	{
		additional_arguments_ = args;
	}

	void set_cmd(std::string const & cmd)
	{
		cmd_ = cmd;
		auto i = cmd_.find_last_of("\\/", std::string::npos, 2);
		if (i != std::string::npos)
		{
			cmd_ = cmd_.substr(i + 1);
		}
	}
private:
	void print_synopsis(std::ostream & stream) const
	{
		stream << "Synopsis:\n";
		stream << std::string(alias_indent, ' ') << (cmd_.empty() ? "cmd" : cmd_);
		list_options(stream);
		list_arguments(stream);
		stream << std::endl;
	}

	void print_options(std::ostream & stream) const
	{
		stream << "Options:\n";
		for (auto && opt : options_)
		{
			std::stringstream buf;
			buf << std::string(alias_indent, ' ') << alias_list(opt);

			if (opt->handler())
				buf << " " << value_placeholder(opt->handler());

			// pad until the end of line
			if (opt->mandatory())
			{
				if ((size_t)buf.tellp() < line_width - 12)
					buf << std::string(line_width - buf.tellp() - 12, ' ');
				buf << " (mandatory)";
			}
			stream << buf.rdbuf() << '\n';

			print_paragraphs(stream, opt->hint());
			stream << '\n';
		}
	}

	void print_paragraphs(std::ostream & stream, std::string const & text) const
	{
		size_t current_index = 0;
		size_t width = line_width - desc_indent;

		while (current_index < text.size())
		{
			std::string_view view(text.data() + current_index, text.size() - current_index);
			stream << std::string(desc_indent, ' ');

			size_t index = view.find_last_of('\n', width);
			if (index == std::string::npos && view.size() <= line_width)
			{
				index = std::min(view.size(), line_width);
			}
			if (index == std::string::npos)
			{
				index = view.find_last_of(" \t", width, 2);
			}

			stream << std::string_view(&text[current_index], index);
			current_index += index + 1;
			stream << '\n';
		}
	}

	void list_arguments(std::ostream & stream) const
	{
		for (auto && arg : positional_arguments_)
		{
			stream << ' ' << value_placeholder(arg->handler());
		}

		if (additional_arguments_)
		{
			// additional arguments are by its nature optional, so we must
			// use square brackets
			stream << " [" << additional_arguments_->handler()->name() << "...]";
		}
	}

	void list_options(std::ostream & stream) const
	{
		for (auto && opt : options_)
		{
			stream << ' ' << option_placeholder(opt);
		}
	}

	std::string cmd_;
	std::vector<option *> options_;
	std::vector<positional_argument *> positional_arguments_;
	positional_argument_list * additional_arguments_;
};

}
#endif // ARGPAR_FORMATTER_H
