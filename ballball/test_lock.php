<?
error_reporting(0);
//ballball's php extension 
//demo �ϥλ���
//**---ball_mutex_lock(int key)---**       lock your php process like pthread_mutex_lock �����P ball_mutex_unlock �f�t�ϥ�
//**---ball_mutex_unlock(int key)---**     unlock your php process like pthread_mutex_unlock


//����ip�̤j�s�u�d��
//php process ������,num�O�n�Ϊ�key��,2�O��key���̤j�s�u��
//���{���|sleep 5��A���ɶ����H�U�{���X�N���|�Q�A������
//start lock key is 987654  and max connection is 1
//check lock
//�ഫip���ƭ�
$ip="201.33.22.11";
$tmp=explode('.',$ip);
var_dump($tmp);
$num=intval($tmp[0]*255*255*255);
$num+=intval($tmp[1]*255*255);
$num+=intval($tmp[2]*255*255);
$num+=intval($tmp[3]*255*255);
echo "num=$num <Br>\r\n";
if(ball_mutex_lock($num,2)){
		echo "start lock<Br>\r\n";
		// do nothing
		sleep(5);
		//unlock
		ball_mutex_unlock(1);
		echo "end lock<Br>\r\n";
}else{
    echo "id 1 has been locked!<Br>\r\n";
}


?>