#pragma once

enum class node_t {
	EMPTY,
	SCOPE,
	UNKNOWN,
	OPEN_BRACE, // {
	CLOSE_BRACE, // }
	OPEN_PARENTHESIS, // (
	CLOSE_PARENTHESIS, // )
	SEMICOLON, // ;
	SCAN, // ?
	PRINT, // print
	IF, // if
	ELSE, // else
	DO, // do
	WHILE, // while
	UNARY_OPERATION,
	BINARY_OPERATION,
	IDENTIFIER, // [a-zA-Z0-9]
	INTEGER_LITERAL, // [0-9]
	BOOL_TRUE, // true
	BOOL_FALSE // false
};

enum class unOp_t {
	LOGICAL_NEGATION, // !
	NEGATION // -
};

enum class binOp_t {
	ASSIGNMENT, // =
	OR, // ||
	AND, // &&
	EQUAL, // ==
	NOT_EQUAL, // !=
	LESS, // <
	LESS_OR_EQUAL, // <=
	GREATER_OR_EQUAL, // >=
	GREATER, // >
	ADDITION, // +
	SUBSTRACTION, // -
	MULTIPLICATION, // *
	DIVISION // /
};
