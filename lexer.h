#pragma once

#include <cctype>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "types_decl.h"

namespace cplr {

	static const std::map<std::string, node_t> keyW_s = {
		{"print", node_t::PRINT},
		{"if", node_t::IF},
		{"else", node_t::ELSE},
		{"do", node_t::DO},
		{"while", node_t::WHILE},
		{"true", node_t::BOOL_TRUE},
		{"false", node_t::BOOL_FALSE}
	};

	static const std::map<std::string, unOp_t> unOp_s = {
		{"!", unOp_t::LOGICAL_NEGATION}
	};

	static const std::map<std::string, binOp_t> binOp_s = {
		{"=", binOp_t::ASSIGNMENT},
		{"||", binOp_t::OR},
		{"&&", binOp_t::AND},
		{"==", binOp_t::EQUAL},
		{"!=", binOp_t::NOT_EQUAL},
		{"<", binOp_t::LESS},
		{"<=", binOp_t::LESS_OR_EQUAL},
		{">=", binOp_t::GREATER_OR_EQUAL},
		{">", binOp_t::GREATER},
		{"+", binOp_t::ADDITION},
		{"-", binOp_t::SUBSTRACTION},
			// in lexical analysis all '-' are recognised as binOp...
		{"*", binOp_t::MULTIPLICATION},
		{"/", binOp_t::DIVISION}
	};

	static std::map<std::string, size_t> IDs;

	class node {
	public:
		node(node_t type);
		virtual ~node();
		node_t get_type() const;
		operator node_t() const;
	protected:
		node_t type_;
	};

	class id final : public node {
	public:
		id(size_t id);
		size_t get_id() const;

	private:
	 size_t id_;
	};

	class intLit final : public node {
	public:
		intLit(int int_lit);
		int get_int() const;
	private:
	 int int_lit_;
	};

	class unOp final : public node {
	public:
		unOp(unOp_t op);
		unOp(const unOp& other);
		unOp_t get_op() const;
	private:
		unOp_t op_;
	};

	class binOp final : public node {
	public:
		binOp(binOp_t op);
		binOp(const binOp& other);
		binOp_t get_op() const;
	private:
		binOp_t op_;
	};

	class unKnw final : public node {
	public:
		unKnw(std::string str);
	 	const std::string& get_str() const;
	private:
		std::string str_;
	};

	template <typename ForwardIterator>
	void skip_spaces(ForwardIterator& cur, ForwardIterator end) {
		while (cur != end && isspace(*cur)) {
			++cur;
		}
	}

	template <typename ForwardIterator>
	std::shared_ptr<node> read_digit_token(
		ForwardIterator& cur, ForwardIterator end
	) {
		std::string token;
		while (cur != end && isdigit(*cur)) {
			token.push_back(*cur);
			++cur;
		}

		std::shared_ptr<node> ptr;
		ptr = std::make_shared<intLit>(stoi(token));
		return ptr;
	}

	template <typename ForwardIterator>
	std::shared_ptr<node> read_id_keyword_token(
		ForwardIterator& cur, ForwardIterator end
	) {
		std::string token;
		while (cur != end && (isalpha(*cur) || isdigit(*cur))) {
			token.push_back(*cur);
			++cur;
		}

		std::shared_ptr<node> ptr;
		auto if_key = keyW_s.find(token);
		if (if_key != keyW_s.end()) {
			ptr = std::make_shared<node>(if_key->second);
			return ptr;
		}
		auto if_id = IDs.find(token);
		if (if_id != IDs.end()) {
			ptr = std::make_shared<id>(if_id->second);
		} else {
			size_t new_id = IDs.size();
			IDs[token] = new_id;
			ptr = std::make_shared<id>(new_id);
		}
		return ptr;
	}

	bool isauxiliary(char c);

	template <typename ForwardIterator>
	std::shared_ptr<node> read_auxiliary_token(ForwardIterator& cur) {
		char c = *(cur++);
		std::shared_ptr<node> ptr;
		if (c == '{') {
			ptr = std::make_shared<node>(node_t::OPEN_BRACE);
		} else if (c == '}') {
			ptr = std::make_shared<node>(node_t::CLOSE_BRACE);
		} else if (c == '(') {
			ptr = std::make_shared<node>(node_t::OPEN_PARENTHESIS);
		} else if (c == ')') {
			ptr = std::make_shared<node>(node_t::CLOSE_PARENTHESIS);
		} else if (c == ';') {
			ptr = std::make_shared<node>(node_t::SEMICOLON);
		} else if (c == '?') {
			ptr = std::make_shared<node>(node_t::SCAN);
		}
		return ptr;
	}

	template <typename ForwardIterator>
	std::shared_ptr<node> read_op_token(
		ForwardIterator& cur, ForwardIterator end
	) {
		std::string token;
		std::shared_ptr<node> ptr;
		token.push_back(*(cur++));

		if (cur != end && ispunct(*cur)) {
			token.push_back(*cur);
			auto if_binop = binOp_s.find(token);
			if (if_binop != binOp_s.end()) {
				ptr = std::make_shared<binOp>(if_binop->second);
				++cur;
				return ptr;
			} else {
				auto if_unop = unOp_s.find(token);
				if (if_unop != unOp_s.end()) {
					ptr = std::make_shared<unOp>(if_unop->second);
					++cur;
					return ptr;
				} else {
					token.pop_back();
				}
			}
		}

		auto if_binop = binOp_s.find(token);
		if (if_binop != binOp_s.end()) {
			ptr = std::make_shared<binOp>(if_binop->second);
		} else {
			auto if_unop = unOp_s.find(token);
			if (if_unop != unOp_s.end()) {
				ptr = std::make_shared<unOp>(if_unop->second);
			} else {
				while (cur != end && ispunct(*cur) && !isauxiliary(*cur)) {
					token.push_back(*(cur++));
				}
				ptr = std::make_shared<unKnw>(token);
			}
		}
		return ptr;
	}

	template <typename ForwardIterator>
	std::shared_ptr<node> read_op_auxiliary_token(
		ForwardIterator& cur, ForwardIterator end
	) {
		std::shared_ptr<node> ptr;
		if (isauxiliary(*cur)) {
			ptr = read_auxiliary_token(cur);
		} else {
			ptr = read_op_token(cur, end);
		}

		return ptr;
	}

	template <typename ForwardIterator>
	std::vector<std::shared_ptr<node>> tokenize(
		ForwardIterator begin, ForwardIterator end
	) {
		std::vector<std::shared_ptr<node>> tokens_list;
		ForwardIterator cur = begin;

		while (cur != end) {
			skip_spaces(cur, end);
			if (isdigit(*cur)) {
				tokens_list.push_back(read_digit_token(cur, end));
			} else if (isalpha(*cur)) {
				tokens_list.push_back(read_id_keyword_token(cur, end));
			} else if (ispunct(*cur)) {
				tokens_list.push_back(read_op_auxiliary_token(cur, end));
			}
		}

		return tokens_list;
	}

}
