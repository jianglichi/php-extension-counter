Building  ballball's php extension

����
step1
tar -xvzf ballball.gz
cd ballball

����phpize
step2:path is your phpize program path
$ path/phpize

�i��configure
step3:
./configure enable-ballball --with-php-config=path/php-config

//make
step4:
make

//make install
step5:
make install

//�ק�php.ini
step5:your php.ini should look something like this:
[ballball]
extension=ballball.so


test.php and test_lock.php is a demo file.