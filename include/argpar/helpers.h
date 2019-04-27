#ifndef ARGPAR_HELPERS_H
#define ARGPAR_HELPERS_H

#include <string>
#include <sstream>

namespace argpar::helpers
{

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


}

 #endif // ARGPAR_HELPERS_H
