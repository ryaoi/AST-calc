#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum keyword {
	_INTEGER,
	_PLUS,
	_UNARYPLUS,
	_UNARYMINUS,
	_MINUS,
	_MUL,
	_DIV,
	_LPAREN,
	_RPAREN,
	_EOF,
};

typedef struct 		s_token {
	int				type;
	int				value;
}					t_token;

typedef struct 		s_lexer {
	char 			*str;
	int				pos;
	char			current_char;
}					t_lexer;


typedef struct		s_ast {
	struct s_ast	*left;
	struct s_ast	*right;
	t_token			*token;
}					t_ast;

typedef struct		s_parser {
	t_lexer			*ptr_lex;
	t_token			*cur_token;
	t_ast			*ptr_ast;
}					t_parser;


t_ast		*exprcalc(t_parser *parser);
void    	freeast(t_ast *ast);

t_token	*get_next_token(t_lexer *lex)
{
	t_token *token;

	if ((!(token = malloc(sizeof(t_token)))))
		exit(EXIT_FAILURE);
	while (lex->pos < strlen(lex->str))
	{
		if (lex->current_char == ' ')
		{
			while (lex->current_char == ' ')
			{
				lex->pos += 1;
				lex->current_char = lex->str[lex->pos];
			}
			continue;
		}
		if (lex->current_char >= '0' && lex->current_char <= '9')
		{
			token->type = _INTEGER;
			token->value = atoi(&(lex->str[lex->pos]));
			int size = 1;
			int value = token->value;
			while (value > 9)
			{
				value /= 10;
				size++;
			}
			lex->pos += size;
			lex->current_char = lex->str[lex->pos];
			return token;
		}

		switch(lex->current_char) {
			case '+':
				token->type = _PLUS;
				token->value = '+';
				break;
			case '-':
				token->type = _MINUS;
				token->value = '-';
				break;
			case '*':
				token->type = _MUL;
				token->value = '*';
				break;
			case '/':
				token->type = _DIV;
				token->value = '/';
				break;
			case '(':
				token->type = _LPAREN;
				token->value = '(';
				break;
			case ')':
				token->type = _RPAREN;
				token->value = ')';
				break;
			default:
				dprintf(2, "Invalid character!:%c\n", lex->current_char);
				exit(EXIT_FAILURE);
		}
		lex->pos++;
		lex->current_char = lex->str[lex->pos];
		return token;

	}
	token->type = _EOF;
	token->value = 0;
	return token;
}

void	eat(t_parser *parser, int type)
{
	if (parser->cur_token->type == type)
	{
		if (type == _LPAREN || type == _RPAREN || type == _EOF)
			free(parser->cur_token);
		if (type != _EOF)
			parser->cur_token = get_next_token(parser->ptr_lex);
	}
	else
	{
		if (parser->ptr_ast != NULL)
			freeast(parser->ptr_ast);
		dprintf(2, "eat failed:%d\n", parser->cur_token->type);
		exit(EXIT_FAILURE);
	}
}

t_ast	*create_ast(t_ast *left, t_ast *right, t_token *token)
{
	t_ast	*new;

	if ((!(new = malloc(sizeof(t_ast)))))
		exit(EXIT_FAILURE);
	new->right = right;
	new->left = left;
	new->token = token;
	return (new);
}

t_ast		*factor(t_parser *parser)
{
	t_token	*token;
	t_ast	*node;

	token = parser->cur_token;
	if (token->type == _MINUS)
	{
		eat(parser, _MINUS);
		token->type = _UNARYMINUS;
		return (create_ast(factor(parser), NULL, token));
	}
	if (token->type == _PLUS)
	{
		eat(parser, _PLUS);
		token->type = _UNARYPLUS;
		return (create_ast(factor(parser), NULL, token));
	}
	if (token->type == _INTEGER)
	{
		eat(parser, _INTEGER);
		return (create_ast(NULL, NULL, token));
	}
	if (token->type == _LPAREN)
	{
		eat(parser, _LPAREN);
		node = exprcalc(parser);
		eat(parser, _RPAREN);
		return node;
	}
	return NULL;

}

t_ast		*term(t_parser *parser)
{
	t_token *token;
	t_ast	*node;

	node = factor(parser);
	while (parser->cur_token->type == _MUL || parser->cur_token->type == _DIV)
	{
		token = parser->cur_token;
		if (parser->cur_token->type == _MUL)
			eat(parser, _MUL);
		else if (parser->cur_token->type == _DIV)
			eat(parser, _DIV);
		node = create_ast(node, factor(parser), token);
	}
	return node;
}

t_ast		*exprcalc(t_parser *parser)
{
	t_token	*token;
	t_ast	*node;

	node = term(parser);
	while (parser->cur_token->type == _PLUS || parser->cur_token->type == _MINUS)
	{
		token = parser->cur_token;
		if (parser->cur_token->type == _PLUS)
			eat(parser, _PLUS);
		else if (parser->cur_token->type == _MINUS)
			eat(parser, _MINUS);
		node = create_ast(node, term(parser), token);
	}
	return node;
}

void	print_ast(t_ast *ast)
{
	if (ast->left != NULL)
		print_ast(ast->left);
	if (ast->right != NULL)
		print_ast(ast->right);
	printf("ast:%p\tast->token:%p\n", ast, ast->token);
	if (ast->token->type != 0)
		printf("type:%dvalue:%c\n", ast->token->type, ast->token->value);
	else
		printf("type:%dvalue:%d\n", ast->token->type, ast->token->value);
}

int		interpret(t_ast *ast)
{
	if (ast->token->type == _UNARYPLUS)
		return 1 * interpret(ast->left);
	else if (ast->token->type == _UNARYMINUS)
		return -1 * interpret(ast->left);
	else if (ast->token->type == _PLUS)
		return interpret(ast->left) + interpret(ast->right);
	else if (ast->token->type == _MINUS)
		return interpret(ast->left) - interpret(ast->right);
	else if (ast->token->type == _MUL)
		return interpret(ast->left) * interpret(ast->right);
	else if (ast->token->type == _DIV)
		return interpret(ast->left) / interpret(ast->right);
	else
		return ast->token->value;
	return 0;
}

void	freeast(t_ast *ast)
{
	if (ast->left != NULL)
		freeast(ast->left);
	if (ast->right != NULL)
		freeast(ast->right);
	free(ast->token);
	free(ast);
}

void	parse(t_lexer *lex)
{
	t_parser	parser;

	parser.ptr_lex = lex;
	parser.cur_token = get_next_token(parser.ptr_lex);
	parser.ptr_ast = NULL;
	parser.ptr_ast = exprcalc(&parser);
	eat(&parser, _EOF);

	printf("result:%d\n", interpret(parser.ptr_ast));
	freeast(parser.ptr_ast);
}


int main()
{
	char buf[200];
	t_lexer lex;	
	t_token token;

	bzero(buf, sizeof(buf));
	fgets(buf, sizeof(buf), stdin);
	*((char *)strchr(buf, '\n')) = '\0';
	lex.str = strdup(buf);
	lex.pos = 0;
	lex.current_char = lex.str[lex.pos];
	parse(&lex);
	free(lex.str);
	return 0;
}
