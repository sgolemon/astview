Simple AST viewer

To use, compile it as a normal php extension:

`$ phpize && ./configure && make`

Then any script invoked while the module is loaded, will dump its AST:

```
$ php -d extension_dir=modules -d extension=astview.so -r 'print "Hello World\n";'
ZEND_AST_STMT_LIST
  ZEND_AST_PRINT
    ZEND_AST_ZVAL string (12) "Hello+World%0A"
Hello World
```

Note that urlencode() is used on ZEND_AST_ZVAL strings to ensure that non-printable characters are displayed sensibly.


