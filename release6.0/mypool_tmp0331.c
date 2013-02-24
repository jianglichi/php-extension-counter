#define	_buflen	        8192                  //bufer長度(不可小於100)
#define	_mytextlen	1024*1024            //struct len must bigger than clobmax
#define	_clobmax	1024*1024            //clob field max len
#define	_fieldlen	4096                 //normal field len
#define	_timechk	10                   //server檢查timeout時間
#define	_rowsbegin	"\r\n\nrowsbegin-->"   
#define	_rowsend	"\r\n<--rowsend"             
#define	_fieldbegin	"\r\nfieldbegin-->\r\n"       
#define	_fieldend	"\r\n<--fieldend"       
#define	_commiton	"set commit on"       
#define	_commitoff	"set commit off"    
#define	_autocommitoff	"auto commit off\r\n"       
#define	_autocommiton	"auto commit on\\rn"       
#define	_ok	        "OK\r\n"       
#define	_commit	        "commit"       
#define	_rollback	"rollback"       
#define	_serverdown	"server down"   
#define	_inputend	"--INPUT END--"
#define	_end	        "--END--"
#define	_output_begin	"--BEGIN--\r\n"
#define	_output_beginerr "--BEGIN ERR--\r\n"
#define	_output_column	"--COLUMN--"
#define	_output_row	"\r\n--ROW--\r\n"
#define	_output_end	"--END--\r\n"

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "oracle_func.h"

void *thread_process();
void sigpipefunc();
int getsession();
void *thread_process_timeout();
void threadbreak();
void read_cfg();
long int data_buffer(mytext *thisdata,char *data,int type,int dsize);
void mystrncat(mytext *thisdata,char *data,int size);
void send_process(char *data,int len,struct pool_oci *ptr);


int _phmax=5,_oracle_conns=5,_timeout=20;
int threadmax;   //max thread
pthread_t *ph;  //thread上限
pthread_t *ph_child;  //thread上限
pthread_t ph_check;
pool_oci *poci;
pthread_mutex_t mylock=PTHREAD_MUTEX_INITIALIZER;
int sock;
long int conn_times=0,conn_times1=0;
time_t t;
struct tm *datetime;

main(int arc,char **argv){
     int port,*ret,option=1;
     int i;
     char *thishost;
     struct hostent* hostdata;
     struct sockaddr_in sadd;
     signal( SIGPIPE, sigpipefunc );

     
     if(arc!=3){
         printf("parater is error\n");
         exit(1);
     }
	
     
     read_cfg();          
	
     thishost=argv[1];
     port=atoi(argv[2]);
     
     if( (hostdata = gethostbyname( thishost )) == NULL ){
         printf("gethostbyname error!!\n");
         exit(1);
     }     
     sadd.sin_family = AF_INET;
     sadd.sin_port   = htons(port);
     sadd.sin_addr   = *(struct in_addr*) hostdata->h_addr;            
     sock = socket( AF_INET, SOCK_STREAM, 0 );
     if(sock==-1){
         printf("socket create error\n");
         exit(1);  
     }     

     setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&option,sizeof( option ) );
     setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)_timeout,sizeof( _timeout ) );

     
     if (bind(sock, (struct sockaddr*) &sadd, sizeof(struct sockaddr_in)) == -1) { 
         perror("(bind) ");  
         exit(1);  
     } 


     if(listen(sock, SOMAXCONN) == -1) { 
         perror("(listen) "); 
         exit(1);   
     }else{
         printf("listening..\n");
     }      	
	
	
     t=time(NULL);
     datetime=localtime(&t);			
	
	 poci=(pool_oci *)malloc(sizeof(pool_oci)*_oracle_conns);
	 ph=(pthread_t *)malloc(sizeof(pthread_t)*_oracle_conns);
	
     for (i=0 ;i<_oracle_conns;i++){
         poci[i].commitmode=0;
         poci[i].busy=0;
         poci[i].datetime=0;
         poci[i].transfer=0;
         poci[i].id=-1;
         poci[i].size=0;
         poci[i].i=i;
		 
	     if(oci_login(&poci[i],host,user,pwd)==-1){
	         printf("oracle login fail!!\n");
	         exit(1);
	     }                  
     }
     
     //pthread_create(&ph_check, NULL, &thread_process_timeout,NULL);  
               
     for (i=0;i<threadmax;i++){
     	   printf("create thread %d\n",i);
         pthread_create(&ph[i], NULL, &thread_process,NULL);
		 sleep(1);
     }
     
     while(1){   
          printf("main program sleep\n");
     	  sleep(10000);
     	  pause();
     }
}

void *thread_process() {
     int thisret,size,i,fields,j,bufout=0,counter=0;
	 int fetch_resu;
     long int datalen,buflen=0;
     int ociposi=-1;
     char buf[_buflen],tmp[_buflen],*realdata ;
     mytext *sqldata;
     int realdatalen=0;
     ub1   *bufp;
     int bufpint=1024;
     dvoid *hdlptr = (dvoid *) 0,*tmphdlptr = (dvoid *) 0;
     ub1   in_out = 0;
     sb2   indptr = 0;
     ub1   piece = OCI_FIRST_PIECE;
     ub4 	hdltype = OCI_HTYPE_DEFINE, iter = 0, idx = 0;
     int poid;
     pthread_detach(pthread_self());
     sqldata=(mytext *)malloc(sizeof(mytext));
     sqldata->next=0;
     sqldata->last=0;
     sqldata->size=0;
     bufp=(ub1 *)malloc(sizeof(ub1)*_clobmax);//**bufp must set to _clobmax
     
     realdata=(char *)malloc(_buflen);  
     realdatalen=_buflen;
     
     if((ociposi=getsession())==-1){
          printf("pool is busy!!\n");
          buf[16]=0;
          exit(1);
     }          
	 
	 
     poci[ociposi].threadid=pthread_self();    
     poci[ociposi].id=ociposi;
     printf("thread id=%d , ociposi=%d \n",pthread_self(),ociposi);
     memset(buf,0,_buflen);
     
			 
     while(1){
          pthread_mutex_lock(&mylock);
          thisret=accept(sock,NULL,NULL);
          pthread_mutex_unlock(&mylock);
          
          poci[ociposi].busy=1;
          bufout=0;
          conn_times++;
          if(conn_times>=1000000){conn_times1++;conn_times=0;}
          if(conn_times1>=1000000)conn_times1=0;
          printf("connections %d millions %d socket=%d ociposi=%d\n",conn_times1,conn_times,thisret,ociposi);
          
          poci[ociposi].datetime=datetime->tm_min*60+datetime->tm_sec;
          poci[ociposi].transfer=0;
          //data_buffer(sqldata,"",2,0);
	        while(sqldata->next){
	            sqldata=sqldata->next;
	        }
	        sqldata->size=0;
	        while(sqldata->last){
	            sqldata=sqldata->last;
	            sqldata->size=0;
	        }								   

          
          
          poci[ociposi].sock=thisret;
          
          
          
          while(size=recv(thisret,buf,_buflen,0)){
               buflen=size;
               buf[buflen]=0;
               poci[ociposi].transfer=1;
               //printf("--buflen=%d \n",buflen);
               
               if(buflen>2){
               }
               else{
                   bufout++;
                   if(bufout>=5 || buflen<=0)break;
               }
               size=buflen;
               if(poci[ociposi].sock==0)continue;
               poci[ociposi].datetime=datetime->tm_min*60+datetime->tm_sec;
               
               
               data_buffer(sqldata,buf,1,buflen);               
               
               //end
               j=0;
               if(size>9)
               for(i=size-9;i<size;i++){
                   tmp[j]=buf[i];
                   j++;
               }
               tmp[j]=0;          
               if((strncmp(buf,_end,strlen(_end))==0)||(strncmp(tmp,_end,strlen(_end))==0)){
                   break;
               }
          
               //set commit off
               if(strncmp(buf,_commitoff,strlen(_commitoff))==0){
                   poci[ociposi].commitmode=1;
                   memset(buf,0,sizeof(buf));
                   send(thisret,_autocommitoff,strlen(_autocommitoff),0);
                   continue;
               }
          
               //_commit
               if(strncmp(buf,_commit,strlen(_commit))==0){
                   OCITransCommit(poci[ociposi].svchp,poci[ociposi].errhp,OCI_DEFAULT );
                   memset(buf,0,sizeof(buf));
                   continue;
               }
                          
               //set commit on
               if(strncmp(buf,_commiton,strlen(_commiton))==0){              
                   poci[ociposi].commitmode=0;
                   memset(buf,0,sizeof(buf));
                   send(thisret,_autocommiton,strlen(_autocommiton),0);
                   continue;
               }                                                                              
               
               //rollback
               if(strncmp(buf,_rollback,strlen(_rollback))==0){
                   OCITransRollback(poci[ociposi].svchp,poci[ociposi].errhp,OCI_DEFAULT) ;
                   poci[ociposi].commitmode=1;
                   memset(buf,0,sizeof(buf));
                   continue;   
               }
               //_serverdown
               if(strncmp(buf,_serverdown,strlen(_serverdown))==0){
               	   OCITerminate(OCI_DEFAULT);
                   exit(1);
               }                    
          
          
               if(sqldata->size+1>=_mytextlen)
               while(sqldata->next){
                  if(sqldata->size+1>=_mytextlen)
                     sqldata=sqldata->next;
                  else
                     break;
               }
               //end input
               size=sqldata->size;
               datalen=size-2; 
               j=0;
               if(size>15)
               for(i=size-15;i<size;i++){
                   tmp[j]=sqldata->mydata[i];
                   j++;
               }
               tmp[j]=0;
               if((strncmp(tmp,_inputend,strlen(_inputend))==0)){    
                   datalen=data_buffer(sqldata,buf,3,0);
                   if(datalen+1>realdatalen){
                       printf("free\n");
                       free(realdata);
                       printf("malloc\n");
                       realdata=(char *)malloc(datalen+1); 
                       realdatalen=datalen+1;
                   }
                   data_buffer(sqldata,realdata,4,0);
                   
                   size=realdatalen;
                   realdata[size-17]=0;          
                   
				   
                   OCIHandleAlloc(poci[ociposi].envhp,(dvoid*)&poci[ociposi].stmthp,OCI_HTYPE_STMT,0,0);
				   
				   printf("oracle_query \n");
                   fields=oracle_query(&poci[ociposi],realdata,datalen-17,tmp);
				   printf("oracle_query ok fields=%d \n",fields);
				   
				   if(fields<=0)break;
                   //printf("start fetch\n");
                   i=0;
				   counter=0;
                   if(fields>0){
                       piece = OCI_FIRST_PIECE ;
                       bufp[0]=0;
                       fetch_data(&poci[ociposi]);
					   counter++;printf("counter=%d \n",counter);
                       send_process(_output_begin,strlen(_output_begin),&poci[ociposi]);
                       
                       while (poci[ociposi].status != OCI_NO_DATA){
               	           //if(OCI_NEED_DATA==poci[ociposi].status){
	               	           //bufpint=4096;
	                           //poci[ociposi].status = OCIStmtGetPieceInfo(poci[ociposi].stmthp, poci[ociposi].errhp, &hdlptr, &hdltype,&in_out, &iter, &idx, &piece);
	                           //poci[ociposi].status = OCIStmtSetPieceInfo(hdlptr, hdltype, poci[ociposi].errhp,(dvoid *) bufp, &bufpint, piece,(CONST dvoid *) &indptr, (ub2 *) 0);
							   
						   //}
						   
						   //printf("data=%s\n",poci[ociposi].data[0]);
						   //printf("data=%s\n",poci[ociposi].data[1]);
						   //printf("data=%s\n",poci[ociposi].data[2]);
						   
						   fetch_data(&poci[ociposi]);
						   if(poci[ociposi].status<0)break;
						   
						   counter++;printf("OCI_NO_DATA=%d , poci[ociposi].status=%d , counter=%d fetch_resu=%d \n",OCI_NO_DATA,poci[ociposi].status,counter);
						   bufp[bufpint]=0;
						   
						   
							for (i=0;i<fields;i++){
								send_process(poci[ociposi].data[i],strlen(poci[ociposi].data[i]),&poci[ociposi]);
								if(i+1<fields){
									send_process(_output_column,strlen(_output_column),&poci[ociposi]);
								}
							}
						    send_process(_output_row,strlen(_output_row),&poci[ociposi]);
						   /*
						   if(poci[ociposi].status==OCI_NEED_DATA){
								if(tmphdlptr!=hdlptr){
								  i++;
							  if(i>1)
							  if((i-1)%fields==0){
								  send_process(_output_row,strlen(_output_row),&poci[ociposi]);
							   }else{
								  send_process(_output_column,strlen(_output_column),&poci[ociposi]);
							   }
							   tmphdlptr=hdlptr;
							   }
							   send_process(bufp,bufpint,&poci[ociposi]);
						   }*/
                       	}
                       	
                       	if(i>1){
                       		send_process(_output_column,strlen(_output_column),&poci[ociposi]);
                       		send_process(bufp,bufpint,&poci[ociposi]);
                       		send_process(_output_row,strlen(_output_row),&poci[ociposi]);
                       	}
                       	send_process(_output_end,strlen(_output_end),&poci[ociposi]);
                   }else{
                      if((fields<0) && (strlen(poci[ociposi].errbuf)>0)){
                      	   send_process(_output_beginerr,strlen(_output_beginerr),&poci[ociposi]);
                           send_process(poci[ociposi].errbuf,512,&poci[ociposi]);
                           send_process(_output_end,strlen(_output_end),&poci[ociposi]);
                      }else{
                           send_process(_ok,strlen(_ok),&poci[ociposi]);
                      }
                   }
                   
                   OCIHandleFree(poci[ociposi].stmthp,OCI_HTYPE_STMT);
                   //printf("fetch ok\n");                                                         
                   send_process("",-1,&poci[ociposi]);
								   //printf("data send ok\n");
						       datalen=0;
								   //data_buffer(sqldata,"",2,0);
						        while(sqldata->next){
						            sqldata=sqldata->next;
						        }
						        sqldata->size=0;
						        while(sqldata->last){
						            sqldata=sqldata->last;
						            sqldata->size=0;
						        }								   
								   //printf("query complete\n");
		               
               }
               poci[ociposi].transfer=0;
          }
          close(thisret);
          poci[ociposi].busy=0;
		  if(fields==-99){
			poci[ociposi].id=-1;
			OCILogoff(poci[ociposi].svchp,poci[ociposi].errhp);
			OCIHandleFree(poci[ociposi].envhp,OCI_HTYPE_ENV);
			
			pthread_create(&ph[ociposi], NULL, &thread_process,NULL);
			printf("exit program \n");
			break;
		  
		  }
    }   

}


void send_process(char *data,int len,struct pool_oci *ptr) {
     char buf[102400];
     if(((ptr->size+len)+1<102400)&& len>=0){
        memcpy(&buf[ptr->size],data,len);
        ptr->size+=len;
     }else{
        if(ptr->size>0){
           buf[ptr->size]=0;
           send(ptr->sock,buf,ptr->size,0);
        }
        if(len>0)
           send(ptr->sock,data,len,0);
        ptr->size=0;
     }
     
     //_buflen
     
}


void sigpipefunc(){
     int i;
     printf("client is break!! reset oracle!!\n");	
     //for (i=0 ;i<_oracle_conns;i++){
         //poci[i].commitmode=0;
         //poci[i].busy=0;
         //oci_login(&poci[i],host,user,pwd);
     //}
     
}



int getsession(){
     int i,j=-1;
     for (i=0 ;i<_oracle_conns;i++){
         if(poci[i].id==-1){
             poci[i].id=0;
             j=i;
             poci[j].commitmode=0;
             break;	
         }
     }     
     return j;
}


void *thread_process_timeout() {
     int i,sum=0,sum1=0;
     
     while(1){
         t=time(NULL);
         datetime=localtime(&t);
         printf("thread checking\n");
     	 for (i=0;i<_oracle_conns;i++){
     	     if((poci[i].busy!=0) && (sum1-poci[i].datetime!=0)){
     	     	 printf ("checking poci[%d].busy=%d\n",i,poci[i].busy);
     	         sum1=datetime->tm_min*60+datetime->tm_sec;
     	         
     	         if((sum1-poci[i].datetime>60)||((sum1-poci[i].datetime)>=_timeout) && (poci[i].transfer==0)){
     	             printf("kill %d\n",i);
     	             //close(poci[i].sock);
     	             pthread_kill(poci[i].threadid,SIGINT);
     	             poci[i].sock=0;
     	             poci[i].busy=0;
     	         }     	         
     	     }
     	 }
         sleep(_timechk);
     }
}



void read_cfg(){
     FILE *fp;
     char tmp[512],tmp1[512];
     int i,j,z,n;
     
     //read config.txt
     if((fp=fopen("mypool_cfg.txt","r"))!=NULL){
     	 for(z=1;z<=6;z++){
	     fgets(tmp1,512,fp);
	     if(z==1)n=6;
	     if(z==2)n=6;
	     if(z==3)n=5;
	     if(z==4)n=11;
	     if(z==5)n=20;
	     if(z==6)n=16;	     
	     j=0;
	     for(i=n;i<strlen(tmp1)-1;i++){
	         if(tmp1[i]==34)break;
	         tmp[j]=tmp1[i];
	         j++;
	     }
	     tmp[j]=0;	     

	     
	     if(z==1){
                host=(char *)malloc(strlen(tmp)+1);
                strcpy(host,tmp);
	     }
	     if(z==2){
                user=(char *)malloc(strlen(tmp)+1);
	     	strcpy(user,tmp);
	     }
	     if(z==3){
                pwd=(char *)malloc(strlen(tmp)+1);
	     	strcpy(pwd,tmp);
	     }
	     if(z==4){
	     	ph=(pthread_t *)malloc(sizeof(pthread_t)*atoi(tmp));
	     	ph_child=(pthread_t *)malloc(sizeof(pthread_t)*atoi(tmp));
	     	threadmax=atoi(tmp);
	     }
	     if(z==5){
	     	_oracle_conns=atoi(tmp);
	     	if(_oracle_conns<threadmax){
	     	    _oracle_conns=threadmax;
	     	    poci=(struct pool_oci *)malloc(sizeof(struct pool_oci)*(threadmax))+1;
	     	}else{
	     	    _oracle_conns=atoi(tmp);
	     	    poci=(struct pool_oci *)malloc(sizeof(struct pool_oci)*atoi(tmp)+1);
	     	}
	     	printf("_oracle_conns=%d\n",_oracle_conns);
	     }
	     if(z==6){
	     	_timeout=atoi(tmp);
	     }

         }
	 fclose(fp);
     }else{
         printf("mypool_cfg.txt error\n");
         fclose(fp);
         exit(1);
     }               

}

long int data_buffer(mytext *thisdata,char *data,int type,int dsize){
     mytext *texttmp,*texttmp1;
     long int size,i;
     char *tmp;
     
     if (type==1){//add
      size=thisdata->size+dsize;
     	
     	if(thisdata->size+1>=_mytextlen)
        while(thisdata->next){
            if(thisdata->size+1>=_mytextlen)
                thisdata=thisdata->next;
            else
                break;
        }
        
        if(size<=_mytextlen-1){
            size=dsize;
            mystrncat(thisdata,data,size);
        }else{
            
      size=_mytextlen-thisdata->size;
	    texttmp=thisdata;
	    

	    if(thisdata->next==0){
	        thisdata=(mytext *)malloc(sizeof(mytext));
	        thisdata->last=texttmp;
	        thisdata->next=0;
	        thisdata->size=0;
	        texttmp->next=thisdata;
	    }else{
	        thisdata=thisdata->next;
	        thisdata->size=0;
	    }
	    thisdata->size=0;
      mystrncat(texttmp,data,size-1);
      tmp=&data[size-1];
      mystrncat(thisdata,tmp,dsize-(size-1));
      }
     	return 0;
     }
     
     if (type==2){//delete
        while(thisdata->next){
            thisdata=thisdata->next;
        }
        thisdata->size=0;
        while(thisdata->last){
            thisdata=thisdata->last;
            thisdata->size=0;
        }

     }

     if (type==3){//get sum
     	size=0;
        while(thisdata->last){
            thisdata=thisdata->last;
        }
     	
     	size=thisdata->size;
        while(thisdata->next){
            thisdata=thisdata->next;
            size=size+thisdata->size;
        }
     	return size;
     }
     
     if (type==4){//get data
     	size=0;
        while(thisdata->last){
            thisdata=thisdata->last;
        }

     	tmp=&data[0];
     	memcpy(tmp,thisdata->mydata,thisdata->size);
     	size=size+thisdata->size;
        while(thisdata->next){
            thisdata=thisdata->next;
            //tmp=&data[size];
            memcpy(&data[size],thisdata->mydata,thisdata->size);
            size=size+thisdata->size;
        }
        data[size]=0;
     }
}


void mystrncat(mytext *thisdata,char *data,int size){
     int i,z;
     //char *tmp;
     z=thisdata->size;
     //tmp=&thisdata->mydata[z];
     memcpy(&thisdata->mydata[z],data,size);
     thisdata->size=thisdata->size+size;
}

