/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2007 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:liki                                                          |
  +----------------------------------------------------------------------+
*/

/* $Id: header,v 1.16.2.1.2.1 2007/01/01 19:32:09 iliaa Exp $ */

#ifndef PHP_BALLBALL_H
#define PHP_BALLBALL_H

extern zend_module_entry ballball_module_entry;
#define phpext_ballball_ptr &ballball_module_entry

#ifdef PHP_WIN32
#define PHP_BALLBALL_API __declspec(dllexport)
#else
#define PHP_BALLBALL_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(ballball);
PHP_MSHUTDOWN_FUNCTION(ballball);
PHP_RINIT_FUNCTION(ballball);
PHP_RSHUTDOWN_FUNCTION(ballball);
PHP_MINFO_FUNCTION(ballball);

PHP_FUNCTION(ball_counter_keyadd);	//auto add value by keys
PHP_FUNCTION(ball_counter_query);   //query key's count
PHP_FUNCTION(ball_counter_list);    //query all key's count
PHP_FUNCTION(ball_counter_remove);  //remove key
PHP_FUNCTION(ball_counter_clear);   //remove and unlock all keys
PHP_FUNCTION(ball_mutex_lock);      //lock your php process like pthread_mutex_lock
PHP_FUNCTION(ball_mutex_unlock);    //unlock your php process like pthread_mutex_unlock

//PHP_FUNCTION(print_array);  //test
//PHP_FUNCTION(hello_array);  //test



#ifdef ZTS
#define BALLBALL_G(v) TSRMG(ballball_globals_id, zend_ballball_globals *, v)
#else
#define BALLBALL_G(v) (ballball_globals.v)
#endif

#endif
