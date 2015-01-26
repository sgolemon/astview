#ifndef incl_ASTVIEW_H
#define incl_ASTVIEW_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"

ZEND_BEGIN_MODULE_GLOBALS(astview)
	zend_bool active;
ZEND_END_MODULE_GLOBALS(astview)

#ifdef ZTS
# define ASTVG(v) ZEND_TSRMG(astview_globals_id, zend_astview_globals *, v)
# ifdef COMPILE_DL_BCMATH
ZEND_TSRMLS_CACHE_EXTERN;
# endif
#else
# define ASTVG(v) (astview_globals.v)
#endif

ZEND_EXTERN_MODULE_GLOBALS(astview)

#endif // incl_ASTVIEW_H
