<?
error_reporting(6143);
//error_reporting(0);
//ballball's php extension 
//demo �ϥλ���
//the key's limit is 1,700,000 �ثe�W����170�U��key
//every key's count limit is 4,000,000,000 �C��key��count�W����40����


//**---ball_counter_keyadd(int key,int cnt)---**   add a key �W�[�@��key�ȡA�d��1-999999999 �u�����ƭ�
//cnt=0��ܦ۰ʥ[1 , �_�h�̩Ҷ񪺭Ȭ�count��
//**---ball_counter_query(int key)---**    query a key �d�ߤ@��key�ȥثe��count��
//**---ball_counter_list()---**            query all key's �d�ߩҦ�key�ȥثe��count��
//**---ball_counter_remove(int key)---**   remove a key �R���@��key�� �� lock key
//**---ball_counter_clear()---**           clear all �R���Ҧ����
//**---ball_mutex_lock(int key)---**       lock your php process like pthread_mutex_lock �����P ball_mutex_unlock �f�t�ϥ�
//**---ball_mutex_unlock(int key)---**     unlock your php process like pthread_mutex_unlock



//demo start


//insert key to hash
//�s�W�@��key�Ȩ�hash
for($i=0;$i<500;$i++){
		$k=rand(5, 100);
		ball_counter_keyadd($k,0);
		$hash[$k]++;
}



//�C�X�ثe�Ҧ�key��
//list all keys
$array= ball_counter_list();


$key=array_keys($array);
$count=count($key);
$sum=0;
echo "list=<br>\r\n";
for($i=0;$i<$count;$i++){
    $k=$key[$i]	;
	  echo "hash memory key $k=".$array[$k]."<br>\r\n";
	  echo "php' count  key $k=".$hash[$k]."<br>\r\n";
	  $sum+=$array[$k];
}
echo "sum=$sum<br>\r\n";

//�d��key�ȥثecount��
//query key's count
$a50=ball_counter_query(50);
$a80=ball_counter_query(80);
echo "key 50 count=$a50 <Br>\r\n";
echo "key 80 count=$a80 <Br>\r\n";


//remove key 50
ball_counter_remove(50);
$a50=ball_counter_query(50);
$a80=ball_counter_query(80);
echo "key 50 count=$a50 <Br>\r\n";
echo "key 80 count=$a80 <Br>\r\n";





?>
