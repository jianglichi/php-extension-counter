Building  ballball's php extension

解壓
step1
tar -xvzf ballball.gz
cd ballball

執行phpize
step2:path is your phpize program path
$ path/phpize

進行configure
step3:
./configure enable-ballball --with-php-config=path/php-config

//make
step4:
make

//make install
step5:
make install

//修改php.ini
step5:your php.ini should look something like this:
[ballball]
extension=ballball.so


test.php and test_lock.php is a demo file.