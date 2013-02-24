/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | ballball's php extension                                             |
  | Author:liki                                                          |
  | email:jianglichi@gmail.com                                           |
  | 2007/08                                                              |
  | version:0.1beta                                                      |
  +----------------------------------------------------------------------+
*/

/* $Id: header,v 1.16.2.1.2.1 2007/01/01 19:32:09 iliaa Exp $ */

#include <pthread.h>
#include <signal.h>


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_ballball.h"
#include "zend.h"
#include <sys/shm.h>


static int le_ballball;


//counter variable
int shm_counter_id,shm_counter_id1;
int block_count;
struct counter_realall *shm_counter_data;
struct counter_head *shm_counter_head;
struct counter_block *shm_counter_block;
struct counter_hb *shm_counter_hb;
struct counter_real *shm_counter_data1;



int pid,size;
char *addr;
int partition=150;
int lock=0,lockkey=0;
void data_init();
int counter_find_block(int key);
int counter_block_add(int block);
int counter_find_block1(int start,int end,int key);
int counter_find_posi1(int block,int start,int end,int key);
int counter_find_block(int key);

void add_data(int block,int sec,int key,int ucnt);
void ball_mcpy(char *a,char *b,int len,int sw);
void ball_lock();
void ball_unlock();
void ball_add_key(int arg,int arg1);
void ball_do_unlock(int key);


//head total=100 bytes
struct counter_head{
	unsigned int blocks;
	unsigned int length;
	unsigned int lockpid;
	//reserve 88 bytes
};

struct counter_block{
	unsigned int max;
	unsigned int min;
	unsigned int addr;
	unsigned int count;
};

struct counter_real{
	unsigned int key;
	unsigned int count;
};

//max data=2,500,000
struct counter_realall{
	struct counter_real data[3500000];
};
//3,500,000/2 = жwе■дWнн
struct counter_hb{
	struct counter_head head;
	struct counter_block block[24000];
};



int counter_realall_len=sizeof(struct counter_realall);
int counter_hb_len=sizeof(struct counter_hb);
int counter_real_len=sizeof(struct counter_real);
int counter_block_len=sizeof(struct counter_block);

zend_function_entry ballball_functions[] = {
	PHP_FE(ball_counter_keyadd,	NULL)
	PHP_FE(ball_counter_query,	NULL)		
	PHP_FE(ball_counter_list,	NULL)		
	PHP_FE(ball_counter_remove,	NULL)		
	PHP_FE(ball_counter_clear,	NULL)		
	PHP_FE(ball_mutex_lock,	NULL)		
	PHP_FE(ball_mutex_unlock,	NULL)		
	
	//PHP_FE(print_array,	NULL)		
	//PHP_FE(hello_array,	NULL)		
	
	
	{NULL, NULL, NULL}	
};


zend_module_entry ballball_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"ballball",
	ballball_functions,
	PHP_MINIT(ballball),
	PHP_MSHUTDOWN(ballball),
	PHP_RINIT(ballball),		
	PHP_RSHUTDOWN(ballball),	
	PHP_MINFO(ballball),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", 
#endif
	STANDARD_MODULE_PROPERTIES
};


#ifdef COMPILE_DL_BALLBALL
ZEND_GET_MODULE(ballball)
#endif



PHP_MINIT_FUNCTION(ballball)
{
	
	shm_counter_id = shmget(0x1020,counter_realall_len,IPC_CREAT| 0777);
	shm_counter_id1 = shmget(0x1021,counter_hb_len,IPC_CREAT| 0777);
	shm_counter_hb=shmat(shm_counter_id1,0,0);
	shm_counter_data=shmat(shm_counter_id,0,0);
	addr=shmat(shm_counter_id1,0,0);
	lock=0;
	shm_counter_head=&shm_counter_hb->head;
	shm_counter_head->lockpid=0;
	return SUCCESS;
}
PHP_MSHUTDOWN_FUNCTION(ballball)
{
	return SUCCESS;
}

PHP_RINIT_FUNCTION(ballball)
{
	
	shm_counter_head=&shm_counter_hb->head;
	pid=getpid();
	//data_init();
	return SUCCESS;
}
PHP_RSHUTDOWN_FUNCTION(ballball)
{
	if(lock==1){
	   ball_do_unlock(lockkey);
	}
	return SUCCESS;
}

PHP_MINFO_FUNCTION(ballball)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "ballball support", "enabled");
	php_info_print_table_row(2, "Author", "liki");
	php_info_print_table_end();
}


PHP_FUNCTION(ball_counter_query){
	long arg;
	int arg_len, len;
	int block,sec;
	int link;
	int size;
	char c[15];
	ball_lock();
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &arg) == FAILURE) {
		return;
	}
	
  block=counter_find_block((arg));
  sec=counter_find_posi(block,(arg));
	shm_counter_block=&shm_counter_hb->block[block];
  link=shm_counter_block->addr;
  shm_counter_data1=&shm_counter_data->data[link+sec];
  if(arg==shm_counter_data1->key){
	  sprintf(c,"%d",shm_counter_data1->count);
  }else{
	  c[0]='0';
	  c[1]=0;
  }
  len=strlen(c);
  ball_unlock();
  RETURN_STRINGL(c, len, 1);
}


PHP_FUNCTION(ball_counter_remove){
	long arg;
	int arg_len, len;
	int block,sec;
	int link;
	int size;
	int count1,posi;
	char c[15];
	char tmp1[500000];
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &arg) == FAILURE) {
		return;
	}
	ball_lock();
	
  block=counter_find_block((arg));
  sec=counter_find_posi(block,(arg));
	shm_counter_block=&shm_counter_hb->block[block];
  link=shm_counter_block->addr;
  shm_counter_data1=&shm_counter_data->data[link+sec];
  count1=shm_counter_block->count;
  
  if(arg==shm_counter_data1->key){
  	  
	    //move data to front
	    len=(count1-sec-1);
	    
	    if(len>0){
			    posi=link+sec+1;
			    memcpy(&tmp1[0],&shm_counter_data->data[posi],len*counter_real_len);
			    memcpy(&shm_counter_data->data[posi-1],&tmp1[0],len*counter_real_len);
			    //is min data
			    if(sec==0){
							shm_counter_data1=&shm_counter_data->data[link];
			    	  shm_counter_block->min=shm_counter_data1->key;
			    }
			}else{
			    //is max data
			    shm_counter_data1=&shm_counter_data->data[link+sec-1];
			    shm_counter_block->max=shm_counter_data1->key;
			}
	    
		  //write to block counter
		  shm_counter_block->count--;
  }
  
  ball_unlock();
}


PHP_FUNCTION(ball_mutex_lock){
	long arg,arg1;
	long len,link,sec,block;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &arg, &arg1) == FAILURE) {
		return;
	}
	
	ball_lock();
  block=counter_find_block((arg));
  sec=counter_find_posi(block,(arg));
	shm_counter_block=&shm_counter_hb->block[block];
  link=shm_counter_block->addr;
  shm_counter_data1=&shm_counter_data->data[link+sec];	
	if(shm_counter_data1->count>=arg1){
		  ball_unlock();
	    ZVAL_LONG(return_value, 0);
      return;
	}
	lock=1;
	lockkey=arg;
  ball_add_key(arg,arg1);
  //ball_unlock();
  ZVAL_LONG(return_value, 1);
  return;
}


PHP_FUNCTION(ball_mutex_unlock){
	long arg;
	long len,link,sec,block;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &arg) == FAILURE) {
		return;
	}
  ball_do_unlock(arg);
}


void ball_do_unlock(int key){
	int block,sec,link;
	//ball_lock();
  block=counter_find_block((key));
  sec=counter_find_posi(block,(key));
	shm_counter_block=&shm_counter_hb->block[block];
  link=shm_counter_block->addr;
  shm_counter_data1=&shm_counter_data->data[link+sec];	
  if(shm_counter_data1->count>=1)
	shm_counter_data1->count--;
	lock=0;
  ball_unlock();

}


PHP_FUNCTION(print_array)
{
	zval *z_array;
	int count, i;
	zval **z_item;
	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &z_array)) {
		return;
	}
	count = zend_hash_num_elements(Z_ARRVAL_P(z_array));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(z_array));
	for (i = 0; i < count; i++) {
		char* key;
		int idx;
		zend_hash_get_current_data(Z_ARRVAL_P(z_array), (void**) &z_item);
		convert_to_string_ex(z_item);
		if (zend_hash_get_current_key(Z_ARRVAL_P(z_array), &key, &idx, 0) == HASH_KEY_IS_STRING) {
			php_printf("array[%s] = %s", key, Z_STRVAL_PP(z_item));
		} else {
			php_printf("array[%d] = %s", idx, Z_STRVAL_PP(z_item));
		}
		zend_hash_move_forward(Z_ARRVAL_P(z_array));
	}
}




PHP_FUNCTION(hello_array)
{
  char *mystr;
  zval *mysubarray;

  array_init(return_value);
  add_index_long(return_value, 42, 123);
  add_next_index_string(return_value, "I should now be found at index 43", 1);
  add_next_index_stringl(return_value, "I'm at 44!", 10, 1);
  mystr = estrdup("Forty Five");
  add_next_index_string(return_value, mystr, 0);
  add_assoc_double(return_value, "pi", 3.1415926535);
  ALLOC_INIT_ZVAL(mysubarray);
  array_init(mysubarray);
  add_next_index_string(mysubarray, "hello", 1);
  add_assoc_zval(return_value, "subarray", mysubarray);    
} 





PHP_FUNCTION(ball_counter_clear){
	ball_lock();
	data_init();
	ball_unlock();
}


PHP_FUNCTION(ball_counter_list)
{
  char *mystr;
  zval *mysubarray;
  int i,j,sum=0,sum_block;
  int addr;
  
  array_init(return_value);      
  ball_lock();
  sum_block = shm_counter_head->blocks;  
  
  for(j=0;j<sum_block;j++){
      shm_counter_block=&shm_counter_hb->block[j];
      sum+=shm_counter_block->count;
		  for(i=0;i<shm_counter_block->count;i++){
		  	  addr=shm_counter_block->addr;
				  shm_counter_data1=&shm_counter_data->data[addr+i];
		      add_index_long(return_value, shm_counter_data1->key, shm_counter_data1->count);
		  }
  }
  ball_unlock();
}

void ball_lock(){
	int startpid,sum;
  //startpid=shm_counter_head->lockpid;
  //sum=0;
  //printf("lockpid=%d pid=%d\n",shm_counter_head->lockpid,pid);
  //mutex_lock(mx_lock);

  if(shm_counter_head->lockpid==0){
	shm_counter_head->lockpid=pid;
	return;
  }
  
  if(shm_counter_head->lockpid==pid){
    //printf("still lock now lockpid=%d pid=%d\n",shm_counter_head->lockpid,pid);
	return;  
  }
  while(shm_counter_head->lockpid!=0){
  	  sleep(1);
  	  printf("wait process %d  unlock\n",shm_counter_head->lockpid);
  	  //sum++;
  	  //if((sum>=5) && (startpid==shm_counter_head->lockpid)){
  	  //    shm_counter_head->lockpid=0;
  	  //}
  }
  shm_counter_head->lockpid=pid;
  //printf(" lock now lockpid=%d pid=%d\n",shm_counter_head->lockpid,pid);
  //mutex_unlock(mx_lock);

  
  
}

void ball_unlock(){
  printf("printf release lock\n");
  shm_counter_head->lockpid=0;	
}



void ball_add_key(int arg,int arg1){
	char *strg;
  int startpid,sum,addr1;
  char tmp1[500000];
  int block,sec,i,j;
  int max,min,count1,addr;
  int link,newblock;
  int sum_block,len;
  	
  block=counter_find_block((arg));
	shm_counter_block=&shm_counter_hb->block[block];
	count1=shm_counter_block->count;
  sec=counter_find_posi(block,(arg));
	max=shm_counter_block->max;
	min=shm_counter_block->min;
	
	while(1){
		  if(count1>=partition){
		  	  sum_block = shm_counter_head->blocks;
		  	  if(sum_block>=35000){ball_unlock();return;}
		  	  
			    //reset old block max data
			    link=shm_counter_block->addr+(partition/2)-1;
			    shm_counter_data1=&shm_counter_data->data[link];
			    shm_counter_block->max=shm_counter_data1->key;
			    //reset old block count data
			    shm_counter_block->count=shm_counter_block->count/2;
			    
			    //move data to new block
			    newblock=counter_block_add(block);	 
			    shm_counter_block=&shm_counter_hb->block[block];   
			    len=partition/2;
			    addr=shm_counter_block->addr;
			    
					memcpy(&tmp1[0],&shm_counter_data->data[addr+len],len*counter_real_len);
					shm_counter_block=&shm_counter_hb->block[newblock];
					addr1=shm_counter_block->addr;
					memcpy(&shm_counter_data->data[addr1],&tmp1[0],len*counter_real_len);
			    
			    
			    //reset new block min data
			    shm_counter_data1=&shm_counter_data->data[addr1];
			    shm_counter_block->min=shm_counter_data1->key;
			    
			    //reset new block max,count data
			    shm_counter_data1=&shm_counter_data->data[addr1+len-1];
			    shm_counter_block->max=shm_counter_data1->key;
			    shm_counter_block->count=partition/2;
			    
			    //block moveing
			    sum_block = shm_counter_head->blocks;
			    len=(sum_block-block-1)*counter_block_len;
			    if(len>counter_block_len){
			    	  memcpy(&tmp1[0],&shm_counter_hb->block[block+1],len);
					    memcpy(&shm_counter_hb->block[block+2],&tmp1[0],len);
			    	  memcpy(&tmp1[0],&shm_counter_hb->block[sum_block],counter_block_len);
					    memcpy(&shm_counter_hb->block[block+1],&tmp1[0],counter_block_len);
					}
			    
				  if(sec>=partition/2){
				  	  block++;
							sec=sec-partition/2;
							break;
				  }else{
				      break;
				  }
				  break;
		  }
		  break;
  }
  
  add_data(block,sec,arg,0);	
	
	
}


PHP_FUNCTION(ball_counter_keyadd)
{
	long arg=0,arg1=0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &arg, &arg1) == FAILURE) {
		return;
	}
	ball_lock();
  ball_add_key(arg,arg1);
  ball_unlock();

}

void add_data(int block,int sec,int key,int ucnt)
{
  int count1,link,i;
  int data,len,posi;
  char tmp1[8000];
  //get block data
	shm_counter_block=&shm_counter_hb->block[block];
  count1=shm_counter_block->count;
  link=shm_counter_block->addr;
  
  //get real data
  shm_counter_data1=&shm_counter_data->data[link+sec];
  data=shm_counter_data1->key;
  
  
  if(!shm_counter_data1->count){
  	  //write now data
  	  shm_counter_data1->key=key;
  	  if(ucnt>0)shm_counter_data1->count=ucnt;
		  else shm_counter_data1->count=1;
  	  shm_counter_block->count++;
		  //is min data
		  if(sec==0)shm_counter_block->min=key;
		  //is max data
		  shm_counter_block->max=key;
		  return;
  }
  
  //compare data
  if(data<key){
  	  sec++;
  }
  
  if(data!=key){
	    //move data to back
	    len=(count1-sec);
	    
	    if(len>0){
			    posi=link+sec;
			    memcpy(&tmp1[0],&shm_counter_data->data[posi],len*counter_real_len);
			    memcpy(&shm_counter_data->data[posi+1],&tmp1[0],len*counter_real_len);
			    //is min data
			    if(sec==0)shm_counter_block->min=key;
			}else{
			    //is max data
			    shm_counter_block->max=key;
			}
	    
	    //write now data
	    shm_counter_data1=&shm_counter_data->data[link+sec];
	    if(ucnt>0)shm_counter_data1->count=ucnt;
  	  else shm_counter_data1->count=1;
	    shm_counter_data1->key=key;
	    
		  //write to block counter
		  shm_counter_block->count++;
		  return;
  }else{
      //write now data counter++
      if(ucnt>0)shm_counter_data1->count=ucnt;
      else shm_counter_data1->count++;
      return;
  }
  

}


int counter_find_posi(int block,int key)
{
	int count;
	shm_counter_block=&shm_counter_hb->block[block];
  if(shm_counter_block->count==0){	  	  
  	  return 0;
  }
  return counter_find_posi1(block,0,shm_counter_block->count-1,key);
}


int counter_find_posi1(int block,int start,int end,int key)
{
	int mid,link,memid;
	mid=(start+end)/2;
	
	if(start==end)return start;
  if(start>end){
  	    return start;
  }
	shm_counter_block=&shm_counter_hb->block[block];
  link=shm_counter_block->addr;
  shm_counter_data1=&shm_counter_data->data[link+mid];
  memid=shm_counter_data1->key;
  
  if(memid==key)return mid;
  if(memid<key)return counter_find_posi1(block,mid+1,end,key);
  if(memid>key){
  	   if(mid==0)return 0;
  	   else return counter_find_posi1(block,start,mid-1,key);    
  }
	return mid;
  
}
		


void data_init()
{
	memset(&shm_counter_hb->head,0,counter_hb_len);
	memset(addr,0,counter_real_len*3500000);
}

int counter_find_block(int key)
{
	if(!shm_counter_head->blocks){
		  counter_block_add(0);
		  return 0;
	}	
	return counter_find_block1(0,shm_counter_head->blocks-1,key);
}



int counter_find_block1(int start,int end,int key)
{
	int mid;
	if(start==end)return start;
	mid=(start+end)/2;
	shm_counter_block=&shm_counter_hb->block[mid];
	if(key>=shm_counter_block->min && key<=shm_counter_block->max)return mid;
	if(key>shm_counter_block->max){
  	   if(mid+1>end)return mid;
  	   return counter_find_block1(mid+1,end,key);
	}else{
       if(mid==0)return mid;
  	   return counter_find_block1(start,mid-1,key);
  }
}




int counter_block_add(int block)
{
	shm_counter_block=&shm_counter_hb->block[shm_counter_head->blocks];
	shm_counter_block->addr=partition*shm_counter_head->blocks;
	shm_counter_head->blocks++;
	shm_counter_head->length=shm_counter_head->length+counter_real_len*partition;
	return shm_counter_head->blocks-1;
}

void ball_mcpy(char *a,char *b,int len,int sw){
	if(sw==1){
		while(len--){
			a[len]=b[len];
		}
	}
	
}