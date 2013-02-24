<?
class oraDBM{
		//export NLS_LANG=AMERICAN_AMERICA.UTF8
		var  $host   =       "192.168.2.171";
		var  $port   =       "90005";
		var  $dbh='';
		var  $resu='';
		var  $posi=0;
		var  $rows=0;
		
		var	$_rowsbegin=	"\r\n\nrowsbegin-->"   ;
		var	$_rowsend=	"\r\n<--rowsend"             ;
		var	$_fieldbegin=	"\r\nfieldbegin-->\r\n"       ;
		var	$_fieldend=	"\r\n<--fieldend"       ;
		var	$_commiton=	"set commit on"       ;
		var	$_commitoff=	"set commit off"    ;
		var	$_autocommitoff=	"auto commit off\r\n"       ;
		var	$_autocommiton=	"auto commit on\\rn"       ;
		var	$_ok=	        "OK\r\n"       ;
		var	$_commit=	        "commit"       ;
		var	$_rollback=	"rollback"       ;
		var	$_serverdown=	"server down"   ;
		var	$_inputend=	"  --INPUT END--\r\n";
		var	$_end=	        "--END--";
		var	$_output_begin=	"--BEGIN--\r\n";
		var	$_output_beginerr= "--BEGIN ERR--\r\n";
		var	$_output_column=	"--COLUMN--";
		var	$_output_row=	"\r\n--ROW--\r\n";
		
		
		function connect() {
			$this->dbh = fsockopen($this->host, $this->port, $errno, $errstr, 30);
			if (!$this->dbh) {
			    echo "$errstr ($errno)<br />\n";
			}
		}
		
		
		function query($sql,$autorollback=1,$rollbackreturn=0,$noexit=''){
			
			
			$date=date("Y-m-d H");
			$date1=date("Y-m-d H:i:s");
			$filename ="/home/www/no9adm/sqllog/{$date}_oracle.txt";
			$handle =fopen($filename, 'a+');
			fwrite($handle, $date1.'--'.$sql."\r\n");
			fclose($handle);
			
			
			
			if(!$this->dbh)$this->connect();
			//echo '<!--';
			//echo $sql.$this->_inputend."--<br><Br>\r\n";
			//echo '-->';
			
			
			fwrite($this->dbh, $sql.$this->_inputend);
			$tmp='';
			$sumcount=0;
			
			$this->posi=0;
			$this->rows=0;
			
			$tmp= fread($this->dbh, 1024000);
			//echo "$tmp <Br>\r\n";
			//echo $tmp."<br>";
			if(eregi('--BEGIN ERR--',$tmp)){
				if($autorollback){
					$this->rollback();
				}
				if($rollbackreturn){
					return $tmp;
				}else{
					if(!$noexit){
						echo '--SQL ERR--';
						echo "<!--$sql-->";
						exit;
					}
					//header("location:/");
					//exit;
				}
			}
			
			$tmp1=explode($this->_output_row,$tmp);
			if($tmp1){
				$tmp1[0]=str_replace($this->_output_begin,'',$tmp1[0]);
				array_pop($tmp1);
				//var_dump($tmp1);
				//echo "<Br><Br>";
				$max=count($tmp1);
				$this->rows=$max;
				$this->resu=null;
				//echo "max=$max <Br>";
				if($max>1)$max--;
				//echo "max=$max <Br>";
				for($i=0;$i<$max;$i++){
					//var_dump(explode($this->_output_column,$tmp1[$i]));
					//echo '<Br><Br>';
					$this->resu[]=explode($this->_output_column,$tmp1[$i]);
					
				}
			}
		}
		
		
		function fetch_row(){
			if($this->rows>$this->posi){
				$p=$this->posi++;
				return $this->resu[$p];
			}
		}
		
		function commit() {
			//echo '--'.$this->_commit.'<Br>';
			fwrite($this->dbh, $this->_commit);
			
		}
		
		
		function rollback() {
			fwrite($this->dbh, $this->_rollback);
		}
		
		function close() {
			fwrite($this->dbh, $this->_end);
		}
		
		
		
		
		
}











?>