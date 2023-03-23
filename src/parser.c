#include "util.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>

enum token_type {
	tok_word,
	tok_redir_in,
	tok_redir_out,
	tok_redir_append,
	tok_ampersand,
	tok_comment,
	tok_eof,
	tok_eol,
	tok_eoc,
	tok_error
};

static int parser_last_tt;

static bool is_wildcard(char);
static const char *parser_current_word(const struct parser *);
static void parser_discard_line(void);

static void parser_append_char(struct parser *, int);
static void parser_append_string(struct parser *, const char *);
static void parser_append_token(struct parser *);

static enum token_type parser_get_token(struct parser *, int);
static enum token_type parser_next_token(struct parser *);
static enum token_type parser_parse(struct parser *, struct command *);

void parser_init(struct parser *pa)
{
	pa->lsiz = pa->osiz = ALLOCSIZ;

	pa->line = xmalloc(pa->lsiz);
	pa->offsets = xmalloc(sizeof(size_t) * pa->osiz);
}

enum parser_status parser_readline(struct parser *pa, struct command *cmd)
{
	enum token_type tt;

	pa->lp = pa->op = 0;
	cmd->fin = cmd->fout = NULL;
	cmd->in_background = false;

	pa->offsets[pa->op++] = 0;
	tt = parser_parse(pa, cmd);

	if (tt == tok_eof)
		return parse_fail;

	if (tt == tok_error) {
		if (parser_last_tt != tok_eol)
			parser_discard_line();

		return parse_error;
	}

	for (size_t i = 0; i < pa->op - 1; i++)
		command_push_arg(cmd, pa->line + pa->offsets[i], i);
	command_push_arg(cmd, NULL, pa->op - 1);

	return tt == tok_eol ? parse_over : parse_ok;
}

static bool is_wildcard(char c)
{
	return strchr("[]{}?~*$^", c) != NULL;
}

static const char *parser_current_word(const struct parser *pa)
{
	return pa->line + pa->offsets[pa->op - 1];
}

static void parser_discard_line(void)
{
	for (int c; (c = getchar()) != EOF && c != '\n'; );
}

static void parser_append_char(struct parser *pa, int c)
{
	if (pa->lp >= pa->lsiz) {
		pa->lsiz *= 2;
		pa->line = xrealloc(pa->line, pa->lsiz);
	}

	pa->line[pa->lp++] = c;
}

static void parser_append_token(struct parser *pa)
{
	if (pa->op >= pa->osiz) {
		pa->osiz *= 2;
		pa->offsets = xrealloc(pa->offsets, sizeof(size_t) * pa->osiz);
	}

	pa->offsets[pa->op++] = pa->lp;
}

static void parser_append_string(struct parser *pa, const char *pattern)
{
	glob_t gl;

	if (glob(pattern, GLOB_NOCHECK | GLOB_TILDE, NULL, &gl) == 0) {
		for (size_t i = 0; i < gl.gl_pathc; i++) {
			for (const char *s = gl.gl_pathv[i]; *s; s++) {
				if (*s == '\\')
					s++;

				parser_append_char(pa, *s);
			}

			parser_append_char(pa, '\0');
			parser_append_token(pa);
		}

		globfree(&gl);
	}
}

static enum token_type parser_get_token(struct parser *pa, int c)
{
	switch (c) {
	case EOF:
		return tok_eof;
	case '#':
		parser_discard_line();
		return tok_comment;
	case ';':
		return tok_eoc;
	case '\n':
		return tok_eol;
	case ' ':
	case '\t':
		return parser_get_token(pa, getchar());
	case '>':
		if ((c = getchar()) == '>')
			return tok_redir_append;
		else
			ungetc(c, stdin);

		return tok_redir_out;
	case '<':
		return tok_redir_in;
	case '&':
		return tok_ampersand;
	default:
		do {
			if (c == '\'')
				while ((c = getchar()) != EOF && c != '\'') {
					if (c == '\n')
						printf("> ");

					if (c == '\\' || is_wildcard(c))
						parser_append_char(pa, '\\');

					parser_append_char(pa, c);
				}
			else if (c == '\\') {
				c = getchar();
				if (is_wildcard(c))
					parser_append_char(pa, '\\');

				parser_append_char(pa, c);
			} else
				parser_append_char(pa, c);
		} while ((c = getchar()) != EOF && !strchr("&<>; \t\n", c));

		parser_append_char(pa, '\0');
		ungetc(c, stdin);

		return tok_word;
	}
}

static enum token_type parser_next_token(struct parser *pa)
{
	return parser_last_tt = parser_get_token(pa, getchar());
}

static enum token_type parser_parse(struct parser *pa, struct command *cmd)
{
	enum token_type tt = parser_next_token(pa);

	switch (tt) {
	case tok_redir_in:
		if (parser_next_token(pa) != tok_word) {
			error("errore di sintassi\n");
			return tok_error;
		}

		if (!command_set_fin(cmd, parser_current_word(pa))) {
			error("%s: file non esistente\n", parser_current_word(pa));
			return tok_error;
		}

		pa->lp = pa->offsets[pa->op - 1];
		break;
	case tok_redir_out:
	case tok_redir_append:
		if (parser_next_token(pa) != tok_word) {
			error("errore di sintassi\n");
			return tok_error;
		}

		if (!command_set_fout(cmd, parser_current_word(pa), tt == tok_redir_append)) {
			error("%s: impossibile aprire il file\n", parser_current_word(pa));
			return tok_error;
		}

		pa->lp = pa->offsets[pa->op - 1];
		break;
	case tok_ampersand:
		cmd->in_background = true;
		return tok_ampersand;
	case tok_word:
		pa->lp = pa->offsets[pa->op - 1];
		parser_append_string(pa, parser_current_word(pa));
		break;
	case tok_comment:
		return tok_eol;
	default:
		return tt;
	}

	return parser_parse(pa, cmd);
}
