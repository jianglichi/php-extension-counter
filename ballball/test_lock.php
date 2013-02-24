<?
error_reporting(0);
//ballball's php extension 
//demo 使用說明
//**---ball_mutex_lock(int key)---**       lock your php process like pthread_mutex_lock 必須與 ball_mutex_unlock 搭配使用
//**---ball_mutex_unlock(int key)---**     unlock your php process like pthread_mutex_unlock


//限制ip最大連線範例
//php process 互斥鎖,num是要用的key值,2是此key的最大連線數
//此程式會sleep 5秒，此時間內以下程式碼將不會被再次執行
//start lock key is 987654  and max connection is 1
//check lock
//轉換ip為數值
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