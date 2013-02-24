<?
error_reporting(6143);
//error_reporting(0);
//ballball's php extension 
//demo 使用說明
//the key's limit is 1,700,000 目前上限為170萬個key
//every key's count limit is 4,000,000,000 每個key的count上限為40億次


//**---ball_counter_keyadd(int key,int cnt)---**   add a key 增加一個key值，範圍1-999999999 只接受數值
//cnt=0表示自動加1 , 否則依所填的值為count值
//**---ball_counter_query(int key)---**    query a key 查詢一個key值目前的count數
//**---ball_counter_list()---**            query all key's 查詢所有key值目前的count數
//**---ball_counter_remove(int key)---**   remove a key 刪除一個key值 或 lock key
//**---ball_counter_clear()---**           clear all 刪除所有資料
//**---ball_mutex_lock(int key)---**       lock your php process like pthread_mutex_lock 必須與 ball_mutex_unlock 搭配使用
//**---ball_mutex_unlock(int key)---**     unlock your php process like pthread_mutex_unlock



//demo start


//insert key to hash
//新增一個key值到hash
for($i=0;$i<500;$i++){
		$k=rand(5, 100);
		ball_counter_keyadd($k,0);
		$hash[$k]++;
}



//列出目前所有key值
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

//查詢key值目前count數
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
