#ifndef ARGPAR_OPTION_H
#define ARGPAR_OPTION_H

#include "value_config.h"

namespace argpar::detail
{

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
		: short_name_(0)
		, long_name_("")
		, hint_(hint)
		, flag_dest_(flag_dest)
		, found_(false)
	{
		if (aliases.size() > 2) throw std::invalid_argument("Too many aliases for option");
		if (aliases.empty()) throw std::invalid_argument("Set of aliases cannot be empty");

		for (auto && alias : aliases)
		{
			if (alias.empty()) throw std::invalid_argument("Alias cannot be zero length.");
			if (alias.size() == 1)
			{
				if (short_name_ != 0) throw std::invalid_argument("Only one short name can be given for an option.");
				short_name_ = alias[0];
			}
			else
			{
				if (!long_name_.empty()) throw std::invalid_argument("Only one long name can be given for an option.");
				long_name_ = std::move(alias);
			}
		}
	}

	argpar::value_config & value_cfg()
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

	char short_name() const
	{
		return short_name_;
	}

	std::string const & long_name() const
	{
		return long_name_;
	}

	std::string const & hint() const
	{
		return hint_;
	}

	void set_found()
	{
		found_ = true;
		if (flag_dest_)
			*flag_dest_ = true;
	}

	bool found() const
	{
		return found_;
	}
private:
	char short_name_;
	std::string long_name_;
	std::string hint_;
	bool * flag_dest_;
	argpar::value_config value_config_;
	bool found_;
};

}

#endif // ARGPAR_OPTION_H
