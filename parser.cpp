#include "parser.h"
//#include "types_decl.h"

namespace cplr {

	parser::node_ast::node_ast(node_t type, node_ast *parent)
	: parent_(parent)
	, type_(type)
	{}

	parser::node_ast::~node_ast() {}

	parser::scope_ast::scope_ast()
	: node_ast(node_t::SCOPE)
	{}

	parser::node_ast::operator node_t() const {
	 return type_;
	}

	parser::id_ast::id_ast(size_t id, node_ast *parent)
	: node_ast(node_t::IDENTIFIER, parent)
	, id_(id)
	{}

	parser::id_ast::id_ast(const cplr::id& nd, node_ast *parent)
	: node_ast(node_t::IDENTIFIER, parent)
	, id_(nd.get_id())
	{}

	parser::intLit_ast::intLit_ast(int int_lit, node_ast *parent)
	: node_ast(node_t::INTEGER_LITERAL, parent)
	, int_lit_(int_lit)
	{}

	parser::intLit_ast::intLit_ast(const cplr::intLit& nd, node_ast *parent)
	: node_ast(node_t::INTEGER_LITERAL, parent)
	, int_lit_(nd.get_int())
	{}

	parser::print_ast::print_ast()
	: node_ast(node_t::PRINT)
	, val_(nullptr)
	{}

	parser::print_ast::print_ast(node_ast *val, node_ast *parent)
	: node_ast(node_t::PRINT, parent)
	, val_(val)
	{}

	parser::while_ast::while_ast(
		node_t type, node_ast *cond, node_ast *body, node_ast *parent
	)
	: node_ast(type, parent)
	, cond_(cond)
	, body_(body)
	{}

	parser::if_ast::if_ast()
	: node_ast(node_t::IF)
	, cond_(nullptr)
	, if_body_(nullptr)
	, else_body_(nullptr)
	{}

	parser::if_ast::if_ast(
			node_ast *cond, node_ast *if_body, node_ast *else_body, node_ast *parent
	)
	: node_ast(node_t::IF, parent)
	, cond_(cond)
	, if_body_(if_body)
	, else_body_(else_body)
	{}

	parser::unOp_ast::unOp_ast(unOp_t op, node_ast *rhs, node_ast *parent)
	: node_ast(node_t::UNARY_OPERATION, parent)
	, op_(op)
	, rhs_(rhs)
	{}

	parser::unOp_ast::unOp_ast(
		const cplr::unOp& nd, node_ast *rhs, node_ast *parent
	)
	: node_ast(node_t::UNARY_OPERATION, parent)
	, op_(nd.get_op())
	, rhs_(rhs)
	{}

	parser::binOp_ast::binOp_ast(
		binOp_t op, node_ast *lhs, node_ast *rhs, node_ast *parent
	)
	: node_ast(node_t::BINARY_OPERATION, parent)
	, op_(op)
	, lhs_(lhs)
	, rhs_(rhs)
	{}

	parser::binOp_ast::binOp_ast(
		const cplr::binOp& nd, node_ast *lhs, node_ast *rhs, node_ast *parent
	)
	: node_ast(node_t::BINARY_OPERATION, parent)
	, op_(nd.get_op())
	, lhs_(lhs)
	, rhs_(rhs)
	{}

	parser::empty_ast::empty_ast()
	: node_ast (node_t::EMPTY)
	{}

	parser::empty_ast::empty_ast(node_ast *parent)
	: node_ast (node_t::EMPTY, parent)
	{}

	parser::parser()
	: root_(new empty_ast())
	, err_ctr(0)
	{}

	parser::~parser() {
		parser_dct_aux(root_);
	}

	void parser::parser_dct_aux(node_ast *nd) {
		if (*nd == node_t::SCOPE) {
			auto nd_scope = reinterpret_cast<scope_ast *>(nd);
			for (
				auto it = nd_scope->nodes_.begin(), ite = nd_scope->nodes_.end(); \
				it != ite; ++it
			) {
				parser_dct_aux(*it);
			}
		} else if (*nd == node_t::PRINT) {
			auto nd_print = reinterpret_cast<print_ast *>(nd);
			parser_dct_aux(nd_print->val_);
		} else if (*nd == node_t::IF) {
			auto nd_if = reinterpret_cast<if_ast *>(nd);
			parser_dct_aux(nd_if->cond_);
			parser_dct_aux(nd_if->if_body_);
			parser_dct_aux(nd_if->else_body_);
		} else if (*nd == node_t::DO || *nd == node_t::WHILE) {
			auto nd_while = reinterpret_cast<while_ast *>(nd);
			parser_dct_aux(nd_while->cond_);
			parser_dct_aux(nd_while->body_);
		} else if (*nd == node_t::UNARY_OPERATION) {
			auto nd_unop = reinterpret_cast<unOp_ast *>(nd);
			parser_dct_aux(nd_unop->rhs_);
		} else if (*nd == node_t::BINARY_OPERATION) {
			auto nd_binop = reinterpret_cast<binOp_ast *>(nd);
			parser_dct_aux(nd_binop->lhs_);
			parser_dct_aux(nd_binop->rhs_);
		}
		delete nd;
	}

	//--------rules--------
	//
	//	program -> stmts
	//	block -> { stmts }
	//	stmts -> o | stmts stmt
	//	stmt -> if ( bool ) stmt else stmt
	//				| if ( bool ) stmt
	//				| do stmt while ( bool ) ;
	//				| while ( bool ) stmt
	//				| block
	//				| prnt ;
	//				| ;
	//	prnt -> print right | right
	//	right -> id = bool | bool
	//	bool -> bool || join | join
	//	join -> join && equality | equality
	//	equality -> equality == rel | equality != rel | rel
	//	rel -> expr < expr | expr <= expr | expr >= expr | expr > expr | expr
	//	expr -> expr + term | expr - term | term
	//	term -> term * unary | term / unary | unary
	//	unary -> !unary | -unary | factor
	//	factor -> ( bool ) | id | int | scn | true | false
	//	id -> [node_t::IDENTIFIER]
	//	int -> [node_t::INTEGER_LITERAL]
	//	scn -> cin >>

	int parser::parse(RandomAccessIt begin, RandomAccessIt end) {
		parser_dct_aux(root_);
		errors.clear();
		err_ctr = 0;
		root_ = parse_stmts(begin, end);
		if (begin != end) {
			++err_ctr;
			errors += "Stray token\n";
		}
		if (err_ctr > 0) {
			std::cerr << errors;
			return -1;
		}
		return 0;
	}

	parser::node_ast * parser::parse_block(
		RandomAccessIt& cur, RandomAccessIt end
	) {
		node_ast *nd; // aka interior node
		if (cur == end) {
			nd = new empty_ast();
		} else if (**cur == node_t::OPEN_BRACE) {
			nd = parse_stmts(++cur, end);
			if (cur == end || **cur != node_t::CLOSE_BRACE) {
				++err_ctr;
				errors += "Expected }\n";
			}
			++cur;
		} else {
			nd = new empty_ast();
			++err_ctr;
			errors += "Expected {\n";
		}

		return nd;
	}

	parser::scope_ast * parser::parse_stmts(
		RandomAccessIt& cur, RandomAccessIt end
	) {
		scope_ast *nd = new scope_ast();
		while (err_ctr == 0 && cur != end && **cur != node_t::CLOSE_BRACE) {
			nd->nodes_.push_back(parse_stmt(cur, end));
		}

		return nd;
	}

	bool parser::if_bool(RandomAccessIt it) {
		return (
			**it == node_t::UNARY_OPERATION || **it == node_t::BINARY_OPERATION \
			|| **it == node_t::IDENTIFIER || **it == node_t::INTEGER_LITERAL \
			|| **it == node_t::SCAN \
			|| **it == node_t::BOOL_TRUE || **it == node_t::BOOL_FALSE \
			|| **it == node_t::OPEN_PARENTHESIS || **it == node_t::CLOSE_PARENTHESIS
		);
	}

	parser::RandomAccessIt parser::find_bool_end(
		RandomAccessIt begin, RandomAccessIt end
	) {
		int ctr = 0; // aka parentheses counter
		RandomAccessIt it;
		for (
			it = begin; \
			it != end && ctr >= 0 && if_bool(it); \
			++it
		) {
			if (**it == node_t::OPEN_PARENTHESIS) {
				++ctr;
			} else if (**it == node_t::CLOSE_PARENTHESIS) {
				--ctr;
				if (ctr < 0) {
					break;
				}
			}
		}

		return it;
	}

	parser::if_ast * parser::handle_if_else(
		RandomAccessIt& cur, RandomAccessIt end
	) {
		if_ast *nd_if;
		if (++cur == end || **cur != node_t::OPEN_PARENTHESIS) {
			++err_ctr;
			errors += "Expected (\n";
		} else {
			++cur;
		}
		auto bool_end = find_bool_end(cur, end);
		nd_if = new if_ast(parse_bool(cur, bool_end));
		nd_if->cond_->parent_ = nd_if;
		cur = bool_end;
		if (cur == end || **cur != node_t::CLOSE_PARENTHESIS) {
			++err_ctr;
			errors += "Expected )\n";
		} else {
			++cur;
		}
		node_ast *nd_if_body = parse_stmt(cur, end);
		nd_if->if_body_ = nd_if_body;
		nd_if_body->parent_ = nd_if;
		node_ast *nd_else_body;
		if (cur != end && **cur == node_t::ELSE) {
			nd_else_body = parse_stmt(++cur, end);
		} else {
			nd_else_body = new empty_ast();
		}
		nd_if->else_body_ = nd_else_body;
		nd_else_body->parent_ = nd_if;

		return nd_if;
	}

	parser::while_ast * parser::handle_do_while(
		RandomAccessIt& cur, RandomAccessIt end
	) {
		while_ast *nd_while = new while_ast(
			node_t::DO, nullptr, parse_stmt(++cur, end)
		);
		nd_while->body_->parent_ = nd_while;
		if (cur == end || **cur != node_t::WHILE) {
			++err_ctr;
			errors += "Expected while\n";
		} else {
			++cur;
		}
		if (cur == end || **cur != node_t::OPEN_PARENTHESIS) {
			++err_ctr;
			errors += "Expected (\n";
		} else {
			++cur;
		}
		auto bool_end = find_bool_end(cur, end);
		node_ast *nd_cond = parse_bool(cur, bool_end);
		nd_while->cond_ = nd_cond;
		nd_cond->parent_ = nd_while;
		cur = bool_end;
		if (cur == end || **cur != node_t::CLOSE_PARENTHESIS) {
			++err_ctr;
			errors += "Expected )\n";
		} else {
			++cur;
		}
		if (cur == end || **cur != node_t::SEMICOLON) {
			++err_ctr;
			errors += "Expected ;\n";
		} else {
			++cur;
		}

		return nd_while;
	}

	parser::while_ast * parser::handle_while(
		RandomAccessIt& cur, RandomAccessIt end
	) {
		while_ast *nd_while;
		if (++cur == end || **cur != node_t::OPEN_PARENTHESIS) {
			++err_ctr;
			errors += "Expected (\n";
		} else {
			++cur;
		}
		auto bool_end = find_bool_end(cur, end);
		nd_while = new while_ast(node_t::WHILE, parse_bool(cur, bool_end));
		nd_while->cond_->parent_ = nd_while;
		cur = bool_end;
		if (cur == end || **cur != node_t::CLOSE_PARENTHESIS) {
			++err_ctr;
			errors += "Expected )\n";
		} else {
			++cur;
		}
		node_ast *nd_body = parse_stmt(cur, end);
		nd_while->body_ = nd_body;
		nd_body->parent_ = nd_while;

		return nd_while;
	}

	parser::node_ast * parser::parse_stmt(
		RandomAccessIt& cur, RandomAccessIt end
	) {
		if (err_ctr > 0) return new empty_ast();

		node_ast *nd;
		if (cur == end) {
			++err_ctr;
			errors += "Expected primary-expression\n";
			nd = new empty_ast();
		} else if (**cur == node_t::IF) {
			nd = handle_if_else(cur, end);
		} else if (**cur == node_t::ELSE) {
			++err_ctr;
			errors += "else without previous if\n";
			nd = new empty_ast();
		} else if (**cur == node_t::DO) {
			nd = handle_do_while(cur, end);
		} else if (**cur == node_t::WHILE) {
			nd = handle_while(cur, end);
		} else if (**cur == node_t::OPEN_BRACE) {
			nd = parse_block(cur, end);
		} else if (**cur == node_t::SEMICOLON) {
			nd = new empty_ast();
			++cur;
		} else {
			nd = parse_prnt(cur, end);
			if (cur == end || **cur != node_t::SEMICOLON) {
				++err_ctr;
				errors += "Expected ;\n";
			} else {
				++cur;
			}
		}

		return nd;
	}

	parser::node_ast * parser::parse_prnt(
		RandomAccessIt& cur, RandomAccessIt end
	) {
		if (err_ctr > 0) return new empty_ast();

		node_ast *nd;
		if (cur == end) {
			++err_ctr;
			errors += \
				"Expected print, assignment, expression, identifier or int constant\n";
			nd = new empty_ast();
		} else if (**cur == node_t::PRINT) {
			auto nd_prnt = new print_ast(parse_right(++cur, end));
			nd_prnt->val_->parent_ = nd_prnt;
			nd = nd_prnt;
		} else {
			nd = parse_right(cur, end);
		}

		return nd;
	}

	parser::node_ast * parser::parse_right(
		RandomAccessIt& cur, RandomAccessIt end
	) {
		if (err_ctr > 0) return new empty_ast();

		node_ast *nd;
		RandomAccessIt bool_end;
		if (cur == end) {
			++err_ctr;
			errors += \
				"Expected assignment, expression, identifier or int constant\n";
			nd = new empty_ast();
		} else if (
			end - cur >= 2 && **next(cur) == node_t::BINARY_OPERATION && \
			(*std::static_pointer_cast<cplr::binOp>(*next(cur))).get_op() \
			== binOp_t::ASSIGNMENT
		) {
			bool_end = find_bool_end(cur + 2, end);
			auto nd_binop = new binOp_ast(
				binOp_t::ASSIGNMENT, \
				parse_id(cur, cur + 1), parse_bool(cur + 2, bool_end)
			);
			nd_binop->lhs_->parent_ = nd_binop;
			nd_binop->rhs_->parent_ = nd_binop;
			nd = nd_binop;
		} else {
			bool_end = find_bool_end(cur, end);
			nd = parse_bool(cur, bool_end);
		}
		cur = bool_end;

		return nd;
	}

	bool parser::if_or(RandomAccessIt it) {
		return **it == node_t::BINARY_OPERATION \
			&& ((*std::static_pointer_cast<cplr::binOp>(*it)).get_op() \
			== binOp_t::OR);
	}

	parser::node_ast * parser::parse_bool(
		RandomAccessIt cur, RandomAccessIt end
	) {
		if (err_ctr > 0) return new empty_ast();

		return parse_op(
			cur, end, [this](RandomAccessIt it) { return if_or(it); }, \
			[this](RandomAccessIt l_cur, RandomAccessIt l_end) {
				return parse_bool(l_cur, l_end);
			}, \
			[this](RandomAccessIt l_cur, RandomAccessIt l_end) {
				return parse_join(l_cur, l_end);
			}
		);
	}

	bool parser::if_and(RandomAccessIt it) {
		return **it == node_t::BINARY_OPERATION \
			&& ((*std::static_pointer_cast<cplr::binOp>(*it)).get_op() \
			== binOp_t::AND);
	}

	parser::node_ast * parser::parse_join(
		RandomAccessIt cur, RandomAccessIt end
	) {
		if (err_ctr > 0) return new empty_ast();

		return parse_op(
			cur, end, [this](RandomAccessIt it) { return if_and(it); }, \
			[this](RandomAccessIt l_cur, RandomAccessIt l_end) {
				return parse_join(l_cur, l_end);
			}, \
			[this](RandomAccessIt l_cur, RandomAccessIt l_end) {
				return parse_equality(l_cur, l_end);
			}
		);
	}

	bool parser::if_equal_not_equal(RandomAccessIt it) {
		return **it == node_t::BINARY_OPERATION \
			&& ((*std::static_pointer_cast<cplr::binOp>(*it)).get_op() \
			== binOp_t::EQUAL \
			|| (*std::static_pointer_cast<cplr::binOp>(*it)).get_op() \
			== binOp_t::NOT_EQUAL);
	}

	parser::node_ast * parser::parse_equality(
		RandomAccessIt cur, RandomAccessIt end
	) {
		if (err_ctr > 0) return new empty_ast();

		return parse_op(
			cur, end, [this](RandomAccessIt it) { return if_equal_not_equal(it); }, \
			[this](RandomAccessIt l_cur, RandomAccessIt l_end) {
				return parse_equality(l_cur, l_end);
			}, \
			[this](RandomAccessIt l_cur, RandomAccessIt l_end) {
				return parse_rel(l_cur, l_end);
			}
		);
	}

	bool parser::if_less_greater(RandomAccessIt it) {
		return **it == node_t::BINARY_OPERATION \
			&& ((*std::static_pointer_cast<cplr::binOp>(*it)).get_op() \
			== binOp_t::LESS \
			|| (*std::static_pointer_cast<cplr::binOp>(*it)).get_op() \
			== binOp_t::LESS_OR_EQUAL \
			|| (*std::static_pointer_cast<cplr::binOp>(*it)).get_op() \
			== binOp_t::GREATER_OR_EQUAL \
			|| (*std::static_pointer_cast<cplr::binOp>(*it)).get_op() \
			== binOp_t::GREATER);
	}

	parser::node_ast * parser::parse_rel(
		RandomAccessIt cur, RandomAccessIt end
	) {
		if (err_ctr > 0) return new empty_ast();

		return parse_op(
			cur, end, [this](RandomAccessIt it) { return if_less_greater(it); }, \
			[this](RandomAccessIt l_cur, RandomAccessIt l_end) {
				return parse_expr(l_cur, l_end);
			}, \
			[this](RandomAccessIt l_cur, RandomAccessIt l_end) {
				return parse_expr(l_cur, l_end);
			}
		);
	}

	bool parser::if_addition_substraction(RandomAccessIt it) {
		return **it == node_t::BINARY_OPERATION \
			&& ((*std::static_pointer_cast<cplr::binOp>(*it)).get_op() \
			== binOp_t::ADDITION \
			|| (*std::static_pointer_cast<cplr::binOp>(*it)).get_op() \
			== binOp_t::SUBSTRACTION);
	}

	parser::node_ast * parser::parse_expr(
		RandomAccessIt cur, RandomAccessIt end
	) {
		if (err_ctr > 0) return new empty_ast();

		return parse_op(
			cur, end, [this](RandomAccessIt it) {
									return if_addition_substraction(it) \
									&& **prev(it) != node_t::UNARY_OPERATION \
									&& **prev(it) != node_t::BINARY_OPERATION;
								}, \
			[this](RandomAccessIt l_cur, RandomAccessIt l_end) {
				return parse_expr(l_cur, l_end);
			}, \
			[this](RandomAccessIt l_cur, RandomAccessIt l_end) {
				return parse_term(l_cur, l_end);
			}
		);
	}

	bool parser::if_multiplication_division(RandomAccessIt it) {
		return **it == node_t::BINARY_OPERATION \
			&& ((*std::static_pointer_cast<cplr::binOp>(*it)).get_op() \
			== binOp_t::MULTIPLICATION || \
			(*std::static_pointer_cast<cplr::binOp>(*it)).get_op() \
			== binOp_t::DIVISION);
	}

	parser::node_ast * parser::parse_term(
		RandomAccessIt cur, RandomAccessIt end
	) {
		if (err_ctr > 0) return new empty_ast();

		return parse_op(
			cur, end, [this](RandomAccessIt l_it) {
									return if_multiplication_division(l_it);
								}, \
			[this](RandomAccessIt l_cur, RandomAccessIt l_end) {
				return parse_term(l_cur, l_end);
			}, \
			[this](RandomAccessIt l_cur, RandomAccessIt l_end) {
				return parse_unary(l_cur, l_end);
			}
		);
	}

	parser::node_ast * parser::parse_unary(
		RandomAccessIt cur, RandomAccessIt end
	) {
		if (err_ctr > 0) return new empty_ast();

		node_ast *nd;
		if (cur == end) {
			++err_ctr;
			errors += \
				"Expected expression in parentheses, identifier or int constant\n";
			nd = new empty_ast();
		} else if (**cur == node_t::UNARY_OPERATION) {
			unOp_t op = (*std::static_pointer_cast<cplr::unOp>(*cur)).get_op();
			if (op == unOp_t::LOGICAL_NEGATION) {
				nd = new unOp_ast(
					unOp_t::LOGICAL_NEGATION, parse_unary(next(cur), end)
				);
			} else {
				++err_ctr;
				errors += "Unknown token\n";
				nd = new empty_ast();
			}
		} else if (**cur == node_t::BINARY_OPERATION) {
			binOp_t op = (*std::static_pointer_cast<cplr::binOp>(*cur)).get_op();
			if (op == binOp_t::SUBSTRACTION) {
				nd = new unOp_ast(unOp_t::NEGATION, parse_unary(next(cur), end));
			} else {
				++err_ctr;
				errors += "Unknown token\n";
				nd = new empty_ast();
			}
		} else {
			nd = parse_factor(cur, end);
		}

		return nd;
	}

	parser::node_ast * parser::parse_factor(
		RandomAccessIt cur, RandomAccessIt end
	) {
		if (err_ctr > 0) return new empty_ast();

		node_ast *nd;
		if (cur == end) {
			++err_ctr;
			errors += \
				"Expected expression in parentheses, identifier or int constant\n";
			nd = new empty_ast();
		} else if (**cur == node_t::OPEN_PARENTHESIS) {
			if (**prev(end) != node_t::CLOSE_PARENTHESIS) {
				nd = parse_bool(next(cur), end);
				++err_ctr;
				errors += "Expected )\n";
			} else {
				nd = parse_bool(next(cur), prev(end));
			}
		} else if (**cur == node_t::IDENTIFIER) {
			nd = parse_id(cur, end);
		} else if (**cur == node_t::INTEGER_LITERAL) {
			nd = parse_int(cur, end);
		} else if (**cur == node_t::SCAN) {
			nd = parse_scan(cur, end);
		} else if (**cur == node_t::BOOL_TRUE) {
			nd = parse_true_false(cur, end);
		} else if (**cur == node_t::BOOL_FALSE) {
			nd = parse_true_false(cur, end);
		} else {
			++err_ctr;
			errors += "Unknown token\n";
			nd = new empty_ast();
		}

		return nd;
	}

	parser::node_ast * parser::parse_id(RandomAccessIt cur, RandomAccessIt end) {
		if (err_ctr > 0) return new empty_ast();

		id_ast *nd_id = new id_ast(*std::static_pointer_cast<cplr::id>(*cur));
		if (next(cur) != end) {
			++err_ctr;
			errors += "Stray token\n";
		}

		return nd_id;
	}

	parser::node_ast * parser::parse_int(
		RandomAccessIt cur, RandomAccessIt end
	) {
		if (err_ctr > 0) return new empty_ast();

		intLit_ast *nd_int = new intLit_ast(
			*std::static_pointer_cast<cplr::intLit>(*cur)
		);
		if (next(cur) != end) {
			++err_ctr;
			errors += "Stray token\n";
		}

		return nd_int;
	}

	parser::node_ast * parser::parse_scan(
		RandomAccessIt cur, RandomAccessIt end
	) {
		if (err_ctr > 0) return new empty_ast();

		node_ast *nd = new node_ast(node_t::SCAN);
		if (next(cur) != end) {
			++err_ctr;
			errors += "Stray token\n";
		}

		return nd;
	}

	parser::node_ast * parser::parse_true_false(
		RandomAccessIt cur, RandomAccessIt end
	) {
		if (err_ctr > 0) return new empty_ast();

		node_ast *nd = new node_ast(**cur);
		if (next(cur) != end) {
			++err_ctr;
			errors += "Stray token\n";
		}

		return nd;
	}

	int parser::run() {
		if (err_ctr > 0) {
			return -1;
		}
		IDs_.clear();
		run_aux(root_, 0);

		if (err_ctr > 0) {
			std::cerr << errors;
			return -2;
		}
		return 0;
	}

	void parser::run_aux(node_ast *nd, size_t scp_nesting_lvl) {
		if (err_ctr > 0) return;

		if (*nd == node_t::SCOPE) {
			IDs_.resize(scp_nesting_lvl + 1);
			auto nd_scope = reinterpret_cast<scope_ast *>(nd);
			for (
				auto it = nd_scope->nodes_.begin(), ite = nd_scope->nodes_.end(); \
				it != ite; ++it
			) {
				run_aux(*it, scp_nesting_lvl + 1);
			}
			IDs_.resize(scp_nesting_lvl);
		} else if (*nd == node_t::IF) {
			run_if(reinterpret_cast<if_ast *>(nd), scp_nesting_lvl);
		} else if (*nd == node_t::DO) {
			run_do(reinterpret_cast<while_ast *>(nd), scp_nesting_lvl);
		} else if (*nd == node_t::WHILE) {
			run_while(reinterpret_cast<while_ast *>(nd), scp_nesting_lvl);
		} else if (*nd != node_t::EMPTY) {
			run_prnt(nd, scp_nesting_lvl);
		}
	}

	void parser::run_if(if_ast *nd, size_t scp_nesting_lvl) {
		if (err_ctr > 0) return;

		IDs_.resize(scp_nesting_lvl + 1);
		bool cond = static_cast<bool>(run_expr(nd->cond_, scp_nesting_lvl + 1));
		IDs_[scp_nesting_lvl].clear();
		if (cond) {
			run_aux(nd->if_body_, scp_nesting_lvl + 1);
		} else {
			run_aux(nd->else_body_, scp_nesting_lvl + 1);
		}
		IDs_.resize(scp_nesting_lvl);
	}

	void parser::run_do(while_ast *nd, size_t scp_nesting_lvl) {
		if (err_ctr > 0) return;

		while (1) {
			IDs_.resize(scp_nesting_lvl + 1);
			run_aux(nd->body_, scp_nesting_lvl + 1);
			IDs_[scp_nesting_lvl].clear();
			if (!static_cast<bool>(run_expr(nd->cond_, scp_nesting_lvl + 1))) {
				break;
			}
		}
		IDs_.resize(scp_nesting_lvl);
	}

	void parser::run_while(while_ast *nd, size_t scp_nesting_lvl) {
		if (err_ctr > 0) return;

		while (1) {
			IDs_.resize(scp_nesting_lvl + 1);
			bool cond = static_cast<bool>(run_expr(nd->cond_, scp_nesting_lvl + 1));
			IDs_[scp_nesting_lvl].clear();
			if (cond) {
				run_aux(nd->body_, scp_nesting_lvl + 1);
			} else {
				break;
			}
		}
		IDs_.resize(scp_nesting_lvl);
	}

	void parser::run_prnt(node_ast *nd, size_t scp_nesting_lvl) {
		if (err_ctr > 0) return;

		if (*nd == node_t::PRINT) {
			int value = run_right(
				reinterpret_cast<print_ast *>(nd)->val_, scp_nesting_lvl
			);
			std::cout << value << '\n';
		} else {
			run_right(nd, scp_nesting_lvl);
		}
	}

	int parser::run_right(node_ast *nd, size_t scp_nesting_lvl) {
		if (err_ctr > 0) return 0;

		if (*nd == node_t::IDENTIFIER) {
			return run_id_lval(reinterpret_cast<id_ast *>(nd), scp_nesting_lvl);
		} else if (*nd == node_t::BINARY_OPERATION) {
			binOp_ast *nd_binop = reinterpret_cast<binOp_ast *>(nd);
			if (nd_binop->op_ == binOp_t::ASSIGNMENT) {
				int& id = run_id_lval(
					reinterpret_cast<id_ast *>(nd_binop->lhs_), scp_nesting_lvl
				);
				id = run_expr(nd_binop->rhs_, scp_nesting_lvl);
				return id;
			}
		}
		return run_expr(nd, scp_nesting_lvl);
	}

	int parser::run_expr(node_ast *nd, size_t scp_nesting_lvl) {
		if (err_ctr > 0) return 0;

		if (*nd == node_t::UNARY_OPERATION) {
			return run_unary(reinterpret_cast<unOp_ast *>(nd), scp_nesting_lvl);
		} else if (*nd == node_t::BINARY_OPERATION) {
			return run_binary(reinterpret_cast<binOp_ast *>(nd), scp_nesting_lvl);
		} else {
			return run_int(nd, scp_nesting_lvl);
		}
	}

	int parser::run_unary(unOp_ast *nd, size_t scp_nesting_lvl) {
		if (err_ctr > 0) return 0;

		if (nd->op_ == unOp_t::LOGICAL_NEGATION) {
			return ! run_expr(nd->rhs_, scp_nesting_lvl);
		} else if (nd->op_ == unOp_t::NEGATION) {
			return - run_expr(nd->rhs_, scp_nesting_lvl);
		}
	}

	int parser::run_binary(binOp_ast *nd, size_t scp_nesting_lvl) {
		if (err_ctr > 0) return 0;

		if (nd->op_ == binOp_t::OR) {
			return run_expr(nd->lhs_, scp_nesting_lvl) \
			|| run_expr(nd->rhs_, scp_nesting_lvl);
		} else if (nd->op_ == binOp_t::AND) {
			return run_expr(nd->lhs_, scp_nesting_lvl) \
			&& run_expr(nd->rhs_, scp_nesting_lvl);
		} else if (nd->op_ == binOp_t::EQUAL) {
			return run_expr(nd->lhs_, scp_nesting_lvl) \
			== run_expr(nd->rhs_, scp_nesting_lvl);
		} else if (nd->op_ == binOp_t::NOT_EQUAL) {
			return run_expr(nd->lhs_, scp_nesting_lvl) \
			!= run_expr(nd->rhs_, scp_nesting_lvl);
		} else if (nd->op_ == binOp_t::LESS) {
			return run_expr(nd->lhs_, scp_nesting_lvl) \
			< run_expr(nd->rhs_, scp_nesting_lvl);
		} else if (nd->op_ == binOp_t::LESS_OR_EQUAL) {
			return run_expr(nd->lhs_, scp_nesting_lvl) \
			<= run_expr(nd->rhs_, scp_nesting_lvl);
		} else if (nd->op_ == binOp_t::GREATER_OR_EQUAL) {
			return run_expr(nd->lhs_, scp_nesting_lvl) \
			>= run_expr(nd->rhs_, scp_nesting_lvl);
		} else if (nd->op_ == binOp_t::GREATER) {
			return run_expr(nd->lhs_, scp_nesting_lvl) \
			> run_expr(nd->rhs_, scp_nesting_lvl);
		} else if (nd->op_ == binOp_t::ADDITION) {
			return run_expr(nd->lhs_, scp_nesting_lvl) \
			+ run_expr(nd->rhs_, scp_nesting_lvl);
		} else if (nd->op_ == binOp_t::SUBSTRACTION) {
			return run_expr(nd->lhs_, scp_nesting_lvl) \
			- run_expr(nd->rhs_, scp_nesting_lvl);
		} else if (nd->op_ == binOp_t::MULTIPLICATION) {
			return run_expr(nd->lhs_, scp_nesting_lvl) \
			* run_expr(nd->rhs_, scp_nesting_lvl);
		} else if (nd->op_ == binOp_t::DIVISION) {
			return run_expr(nd->lhs_, scp_nesting_lvl) \
			/ run_expr(nd->rhs_, scp_nesting_lvl);
		}
	}

	int& parser::run_id_lval(id_ast *nd, size_t scp_nesting_lvl) {
		std::map<size_t, int>::iterator id;
		for (auto it = IDs_.begin(), ite = IDs_.end(); it != ite; ++it) {
			id = (*it).find(nd->id_);
			if (id != (*it).end()) {
				return id->second;
			}
		}
		return IDs_[scp_nesting_lvl - 1][nd->id_];
	}

	int parser::run_int(node_ast *nd, size_t scp_nesting_lvl) {
		if (*nd == node_t::IDENTIFIER) {
			auto id = reinterpret_cast<id_ast *>(nd)->id_;
			std::map<size_t, int>::iterator id_val;
			for (auto it = IDs_.begin(), ite = IDs_.end(); it != ite; ++it) {
				id_val = (*it).find(id);
				if (id_val != (*it).end()) {
					return id_val->second;
				}
			}
			++err_ctr;
			errors += "Undefined identifier\n";
			return 0;
		} else if (*nd == node_t::INTEGER_LITERAL) {
			return reinterpret_cast<intLit_ast *>(nd)->int_lit_;
		} else if (*nd == node_t::SCAN) {
			int ret = 0;
			std::cin >> ret;
			return ret;
		} else if (*nd == node_t::BOOL_TRUE) {
			return 1;
		} else { // *nd == node_t::BOOL_FALSE
			return 0;
		}
	}

}
