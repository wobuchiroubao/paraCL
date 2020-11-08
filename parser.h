#pragma once

#include <algorithm>
#include <exception>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "lexer.h"
#include "types_decl.h"

namespace cplr {

	class parser final {
	private:
		class node_ast { // node of abstract syntax tree
		public:
			node_ast(node_t type, node_ast *parent = nullptr);
			virtual ~node_ast();
			operator node_t() const;

			node_ast *parent_;
			node_t type_;
		};

		class scope_ast final : public node_ast {
		public:
			scope_ast();

			std::vector<node_ast *> nodes_;
		};

		class id_ast final : public node_ast {
		public:
			id_ast(size_t id, node_ast *parent = nullptr);
			id_ast(const cplr::id& nd, node_ast *parent = nullptr);

		 size_t id_;
		};

		class intLit_ast final : public node_ast {
		public:
			intLit_ast(int int_lit, node_ast *parent = nullptr);
			intLit_ast(const cplr::intLit& nd, node_ast *parent = nullptr);

		 int int_lit_;
		};

		class print_ast final : public node_ast {
		public:
			print_ast();
			print_ast(node_ast *val, node_ast *parent = nullptr);

			node_ast *val_;
		};

		class while_ast final : public node_ast {
		public:
			while_ast(
				node_t type, node_ast *cond = nullptr, node_ast *body = nullptr, \
				node_ast *parent = nullptr
			);

			node_ast *cond_, *body_;
		};

		class if_ast final : public node_ast {
		public:
			if_ast();
			if_ast(
				node_ast *cond, node_ast *if_body = nullptr, \
				node_ast *else_body = nullptr, node_ast *parent = nullptr
			);

			node_ast *cond_, *if_body_, *else_body_;
		};

		class unOp_ast final : public node_ast {
		public:
			unOp_ast(unOp_t op, node_ast *rhs = nullptr, node_ast *parent = nullptr);
			unOp_ast(
				const cplr::unOp& nd, node_ast *rhs = nullptr, \
				node_ast *parent = nullptr
			);

			unOp_t op_;
			node_ast *rhs_;
		};

		class binOp_ast final : public node_ast {
		public:
			binOp_ast(
				binOp_t op, node_ast *lhs = nullptr, node_ast *rhs = nullptr, \
				node_ast *parent = nullptr
			);
			binOp_ast(
				const cplr::binOp& nd, node_ast *lhs = nullptr, node_ast *rhs = nullptr, \
				node_ast *parent = nullptr
			);

			binOp_t op_;
			node_ast *lhs_, *rhs_;
		};

		class empty_ast final : public node_ast {
		public:
			empty_ast();
			empty_ast(node_ast *parent);
		};

		using RandomAccessIt = std::vector<std::shared_ptr<cplr::node>>::iterator;
		using Elem = std::shared_ptr<cplr::node>;

	public:
		parser();
		~parser();
		int parse(RandomAccessIt begin, RandomAccessIt end);
		int run();

	private:
		void parser_dct_aux(node_ast *nd);

		node_ast * parse_block(RandomAccessIt& cur, RandomAccessIt end);
		scope_ast * parse_stmts(RandomAccessIt& cur, RandomAccessIt end);
		if_ast * handle_if_else(RandomAccessIt& cur, RandomAccessIt end);
		while_ast * handle_do_while(RandomAccessIt& cur, RandomAccessIt end);
		while_ast * handle_while(RandomAccessIt& cur, RandomAccessIt end);
		node_ast * parse_stmt(RandomAccessIt& cur, RandomAccessIt end);
		node_ast * parse_prnt(RandomAccessIt& cur, RandomAccessIt end);
		node_ast * parse_right(RandomAccessIt& cur, RandomAccessIt end);
		node_ast * parse_bool(RandomAccessIt cur, RandomAccessIt end);
		node_ast * parse_join(RandomAccessIt cur, RandomAccessIt end);
		node_ast * parse_equality(RandomAccessIt cur, RandomAccessIt end);
		node_ast * parse_rel(RandomAccessIt cur, RandomAccessIt end);
		node_ast * parse_expr(RandomAccessIt cur, RandomAccessIt end);
		node_ast * parse_term(RandomAccessIt cur, RandomAccessIt end);
		node_ast * parse_unary(RandomAccessIt cur, RandomAccessIt end);
		node_ast * parse_factor(RandomAccessIt cur, RandomAccessIt end);
		node_ast * parse_id(RandomAccessIt cur, RandomAccessIt end);
		node_ast * parse_int(RandomAccessIt cur, RandomAccessIt end);
		node_ast * parse_scan(RandomAccessIt cur, RandomAccessIt end);
		node_ast * parse_true_false(RandomAccessIt cur, RandomAccessIt end);

		bool if_bool(RandomAccessIt it);
		RandomAccessIt find_bool_end(RandomAccessIt begin, RandomAccessIt end);
		template <typename UnaryPredicate, typename Func1, typename Func2>
		node_ast * parse_op(
			RandomAccessIt cur,	RandomAccessIt end, UnaryPredicate if_smth, \
			Func1 parse_lhs, Func2 parse_rhs
		);
		template <typename UnaryPredicate>
		RandomAccessIt find_op_not_in_parentheses(
			RandomAccessIt begin,	RandomAccessIt end, UnaryPredicate if_smth
		);
		bool if_or(RandomAccessIt it);
		bool if_and(RandomAccessIt it);
		bool if_equal_not_equal(RandomAccessIt it);
		bool if_less_greater(RandomAccessIt it);
		bool if_addition_substraction(RandomAccessIt it);
		bool if_multiplication_division(RandomAccessIt it);

		void run_aux(node_ast *nd, size_t scp_nesting_lvl);
		void run_if(if_ast *nd, size_t scp_nesting_lvl);
		void run_do(while_ast *nd, size_t scp_nesting_lvl);
		void run_while(while_ast *nd, size_t scp_nesting_lvl);
		void run_prnt(node_ast *nd, size_t scp_nesting_lvl);
		int run_right(node_ast *nd, size_t scp_nesting_lvl);
		int run_expr(node_ast *nd, size_t scp_nesting_lvl);
		int run_unary(unOp_ast *nd, size_t scp_nesting_lvl);
		int run_binary(binOp_ast *nd, size_t scp_nesting_lvl);
		int& run_id_lval(id_ast *nd, size_t scp_nesting_lvl);
		int run_int(node_ast *nd, size_t scp_nesting_lvl);

		node_ast *root_;
		std::vector<std::map<size_t, int>> IDs_; // scoped identifiers
		std::string errors;
		size_t err_ctr;
	};

	template <typename UnaryPredicate, typename Func1, typename Func2>
	parser::node_ast * parser::parse_op(
		RandomAccessIt cur,	RandomAccessIt end, UnaryPredicate if_smth, \
		Func1 parse_lhs, Func2 parse_rhs
	) {
		node_ast *nd;
		auto it = find_op_not_in_parentheses(cur, end, if_smth);
		if (cur == end) {
			++err_ctr;
			errors += \
				"Expected expression, identifier or int constant\n";
			nd = new empty_ast();
		} else if (it != cur && if_smth(it)) {
			auto nd_binop = new binOp_ast(
				*std::static_pointer_cast<cplr::binOp>(*it), \
				parse_lhs(cur, it), parse_rhs(next(it), end)
			);
			nd_binop->lhs_->parent_ = nd_binop;
			nd_binop->rhs_->parent_ = nd_binop;
			nd = nd_binop;
		} else {
			nd = parse_rhs(cur, end);
		}

		return nd;
	}

	template <typename UnaryPredicate>
	parser::RandomAccessIt parser::find_op_not_in_parentheses(
		RandomAccessIt begin,	RandomAccessIt end, UnaryPredicate if_smth
	) {
		if (begin == end) {
			return end;
		}
		auto it = end;
		int ctr = 0; // aka parenteses counter
		do {
			--it;
			if (**it == node_t::OPEN_PARENTHESIS) {
				--ctr;
			} else if (**it == node_t::CLOSE_PARENTHESIS) {
				++ctr;
			}
		} while (it != begin && !(ctr == 0 && if_smth(it)));

		return it;
	}

}
