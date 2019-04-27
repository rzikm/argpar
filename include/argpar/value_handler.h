#ifndef ARGPAR_VALUE_HANDLER_H
#define ARGPAR_VALUE_HANDLER_H

#include <string>
#include <iostream>

namespace argpar::detail
{

class value_handler
{
public:
	virtual void parse(std::string const & value) const = 0;
	virtual bool has_default() const = 0;
	virtual void set_default() const = 0;
	virtual void print_default(std::ostream & stream) const = 0;
	virtual std::string const & name() const = 0;

	virtual ~value_handler()
	{
	}
};

template<typename TConfig>
class value_handler_base : public value_handler
{
public:
	value_handler_base(std::string const & name)
		: name_ (name)
	{
	}

	virtual bool has_default() const override
	{
		return config_.has_default();
	}

	virtual void print_default(std::ostream & stream) const override
	{
		stream << config_.get_default();
	}

	virtual std::string const & name() const override
	{
		return name_;
	}

	TConfig & get_config()
	{
		return config_;
	}

	TConfig const & get_config() const
	{
		return config_;
	}
private:
	TConfig config_;
	std::string name_;
};

template<typename TConfig>
class single_value_handler : public value_handler_base<TConfig>
{
	using value_type = typename TConfig::value_type;
	using base = value_handler_base<TConfig>;
public:
	single_value_handler(std::string name, value_type * dest)
		: base(name)
		, dest_(dest)
	{
	}

	void parse(std::string const & value) const override
	{
		*dest_ = base::get_config().parse(value);
	}

	virtual void set_default() const override
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
	multi_value_handler(std::string name, container * dest)
		: base(name)
		, dest_(dest)
	{
	}
	void parse(std::string const & value) const override
	{
		dest_->emplace_back(base::get_config().parse(value));
	}

	virtual void set_default() const override
	{ // do nothing
	}

	container * dest_;
};

}
#endif // ARGPAR_VALUE_HANDLER_H
