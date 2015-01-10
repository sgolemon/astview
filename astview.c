/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Sara Golemon <pollita@php.net>                               |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "zend_ast.h"
#include "ext/standard/info.h"
#include "ext/standard/url.h"

#if PHP_MAJOR_VERSION < 7
# error AstView requires PHP version 7 or later
#endif

const char* astview_kindName(zend_ast_kind kind) {
	switch (kind) {
#define AST(id) case ZEND_AST_##id: return "ZEND_AST_" #id;
#define AST_DECL(id) AST(id)
#define AST_LIST(id) AST(id)
#define AST_CHILD(id, children) AST(id)
#include "ast-names.h"
#undef AST_CHILD
#undef AST_LIST
#undef AST_DECL
#undef AST
		default: return "(unknown)";
	}
}

static void astview_visit(zend_ast* ast, int indent);

inline astview_header(zend_ast* ast, int indent) {
	while (indent--) {
		php_printf("  ");
	}
	php_printf("%s", astview_kindName(ast->kind));
	if (ast->attr) {
		php_printf("-attr(%04x)", (int)ast->attr);
	}
}

static void astview_visit_ZVAL(zend_ast* ast, int indent) {
	zval* zv = zend_ast_get_zval(ast);
	astview_header(ast, indent);
	php_printf(" ");
	switch (Z_TYPE_P(zv)) {
		case IS_NULL: php_printf("NULL"); break;
		case IS_FALSE: php_printf("bool(false)"); break;
		case IS_TRUE: php_printf("bool(true)"); break;
		case IS_LONG: php_printf("int(" ZEND_LONG_FMT ")", Z_LVAL_P(zv)); break;
		case IS_DOUBLE: php_printf("float(%.*G)", (int)EG(precision), Z_DVAL_P(zv)); break;
		case IS_STRING: {
			zend_string* encoded = php_url_encode(Z_STRVAL_P(zv), Z_STRLEN_P(zv));
			php_printf("string (%d) \"%s\"", Z_STRLEN_P(zv), encoded->val);
			zend_string_free(encoded);
			break;
		}
		/* The rest of the types don't really happen in ASTs */
		default: php_printf("type(%d)", (int)Z_TYPE_P(zv));
	}
	php_printf("\n");
}

static void astview_visit_list(zend_ast* ast, int indent) {
	astview_header(ast, indent);
	php_printf("\n");

	zend_ast_list* list = zend_ast_get_list(ast);
	uint32_t i;
	for (i = 0; i < list->children; ++i) {
		astview_visit(list->child[i], indent + 1);
	}
}

static void astview_visit_child(zend_ast* ast, int indent, int children) {
	uint32_t i = 0;
	astview_header(ast, indent);
	php_printf("\n");
	for (i = 0; i < children; ++i) {
		astview_visit(ast->child[i], indent + 1);
	}
}

static void astview_visit_decl(zend_ast* ast, int indent) {
	zend_ast_decl *decl = (zend_ast_decl *) ast;
	int i;

	astview_header(ast, indent);
	if (ast->kind != ZEND_AST_CLOSURE) {
		php_printf(" %s%s",
		           decl->name->val,
		           ast->kind == ZEND_AST_CLASS ? "" : "()");
	}
	if (decl->flags) {
		php_printf(" flags(%04x)", (int)decl->flags);
	}
	php_printf("\n");
	for (i = 0; i < 3; ++i) {
		astview_visit(decl->child[i], indent + 1);
	}
}

static void astview_visit_unexpected(zend_ast* ast, int indent) {
	astview_header(ast, indent);
	php_printf(" **UNEXPECTED AST NODE**", astview_kindName(ast->kind));
	php_printf("\n");
}

/* ZNODE is a special AST node which only appears in the compile stage */
#define astview_visit_ZNODE astview_visit_unexpected

static void astview_visit(zend_ast* ast, int indent) {
	if (!ast) return;
	switch (ast->kind) {
#define AST(id) case ZEND_AST_##id: astview_visit_##id(ast, indent); return;
#define AST_DECL(id) case ZEND_AST_##id: astview_visit_decl(ast, indent); return;
#define AST_LIST(id) case ZEND_AST_##id: astview_visit_list(ast, indent); return;
#define AST_CHILD(id, children) case ZEND_AST_##id: astview_visit_child(ast, indent, children); return;
#include "ast-names.h"
#undef AST_CHILD
#undef AST_LIST
#undef AST_DECL
#undef AST
		default:
			astview_visit_unexpected(ast, indent);
	}
}

static zend_ast_process_t previous_ast_process = NULL;
static void astview_ast_process(zend_ast* ast) {
	astview_visit(ast, 0);
	if (previous_ast_process) {
		previous_ast_process(ast);
	}
}

/* {{{ PHP_MINI_FUNCTION */
PHP_MINIT_FUNCTION(astview) {
	previous_ast_process = zend_ast_process;
	zend_ast_process = astview_ast_process;

	return SUCCESS;
} /* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(astview) {
	zend_ast_process = previous_ast_process;
	return SUCCESS;
} /* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(astview) {
	php_info_print_table_start();
	php_info_print_table_row(2, "astview support", "enabled");
	php_info_print_table_end();
} /* }}} */

/* {{{ astview_module_entry
 */
zend_module_entry astview_module_entry = {
	STANDARD_MODULE_HEADER,
	"astview",
	NULL, /* functions */
	PHP_MINIT(astview),
	PHP_MSHUTDOWN(astview),
	NULL, /* RINIT */
	NULL, /* RSHUTDOWN */
	PHP_MINFO(astview),
	"7.0.0-dev",
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_ASTVIEW
ZEND_GET_MODULE(astview)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
