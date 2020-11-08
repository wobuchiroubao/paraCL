#include "lexer.h"

namespace cplr {

	node::node(node_t type)
	: type_(type)
	{}

	node::~node() {}

	node_t node::get_type() const {
		return type_;
	}

	node::operator node_t() const {
		return type_;
	}

	id::id(size_t id)
	: node(node_t::IDENTIFIER)
	, id_(id)
	{}

	size_t id::get_id() const {
		return id_;
	}

	intLit::intLit(int int_lit)
	: node(node_t::INTEGER_LITERAL)
	, int_lit_(int_lit)
	{}

	int intLit::get_int() const {
		return int_lit_;
	}

	unOp::unOp(unOp_t op)
	: node(node_t::UNARY_OPERATION)
	, op_(op)
	{}

	unOp::unOp(const unOp& other)
	: node(other.type_)
	, op_(other.op_)
	{}


	unOp_t unOp::get_op() const {
		return op_;
	}

	binOp::binOp(binOp_t op)
	: node(node_t::BINARY_OPERATION)
	, op_(op)
	{}

	binOp::binOp(const binOp& other)
	: node(other.type_)
	, op_(other.op_)
	{}

	binOp_t binOp::get_op() const {
		return op_;
	}

	unKnw::unKnw(std::string str)
	: node(node_t::UNKNOWN)
	, str_(std::move(str))
	{}

	const std::string& unKnw::get_str() const {
		return str_;
	}

	bool isauxiliary(char c) {
		return (
			c == '{' || c == '}' || c == '(' || c == ')' || c == ';' || c == '?'
		);
	}

}
