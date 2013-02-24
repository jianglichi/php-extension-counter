#include <oci.h>
char *host,*user,*pwd;

typedef struct pool_oci{
        OCIEnv *envhp;
        OCIError *errhp;
        OCISvcCtx *svchp;
        OCIStmt *stmthp;
        OCIDefine *defnp;
        OCIServer   *server;
        int type[100];
        sb4 errcode;
        text errbuf[4096];
        text data[70][4096];
        text fieldname[70][4096];
        long int len;        
        sb4 status;
        int commitmode;  //autocommit=0;set commit on=1;set commit off=2;
        int busy;
        int transfer;
        int datetime;
        int sock;
        int id;
        int size;
		int i;
        pthread_t threadid;
}pool_oci;


typedef struct mytext{
        char mydata[_mytextlen];
        struct mytext *last;
        struct mytext *next;
        int size;
}mytext;


int oci_login(pool_oci *ptr,char _ocihost[],char _ociuser_name[],char _ociuser_pwd[]){
     ptr->errbuf[0]=0;
     
         
     OCIEnvCreate(&ptr->envhp, OCI_THREADED,0,0,0,0,0,0);
     OCIHandleAlloc(ptr->envhp,(dvoid*)&ptr->errhp, OCI_HTYPE_ERROR, 0,0);
     OCILogon(ptr->envhp, ptr->errhp, &ptr->svchp, _ociuser_name, (ub4)strlen ((char *)_ociuser_name),_ociuser_pwd, (ub4)strlen ((char *)_ociuser_pwd), _ocihost, (ub4)strlen((char *)_ocihost));
     //OCILogon(ptr->envhp,ptr->errhp,&ptr->svchp,_ociuser_name,strlen(_ociuser_name),_ociuser_pwd,strlen(_ociuser_pwd), ( text * ) _ocihost, ( ub4 ) strlen( _ocihost));
     OCIErrorGet( ptr->errhp, ( ub4 ) 1, ( text * ) 0,
                   &ptr->errcode, ptr->errbuf, ( ub4 ) sizeof( ptr->errbuf ),
                   OCI_HTYPE_ERROR );
                   
     OCIErrorGet( ptr->errhp, ( ub4 ) 1, ( text * ) 0,
                   &ptr->errcode, ptr->errbuf, ( ub4 ) sizeof( ptr->errbuf ),
                   OCI_HTYPE_ERROR );
     printf("msg:## OCILogon %d %s \n", ptr->status, ptr->errbuf);
     //OCIHandleAlloc(ptr->envhp,(dvoid*)&ptr->stmthp,OCI_HTYPE_STMT,0,0);
   

        
     //OCIErrorGet( ptr->errhp, ( ub4 ) 1, ( text * ) 0,
     //              &ptr->errcode, ptr->errbuf, ( ub4 ) sizeof( ptr->errbuf ),
     //             OCI_HTYPE_ERROR);
                   
                   
                   
                       
     if(strlen(ptr->errbuf)>0){
     	return -1;
     }
     return 1;
                   
}



int oracle_query(pool_oci *ptr,text *sql,int sqllen,char *resu){
     int num_cols=0,i,rows=0,stmttype=0,commitmode=0,status=0;     
	 long int errorcode;
	 int poid;
	 text* ColumnName;
	 ub4 ColumnNameLength;
	 void* param = (void *)0;
	 ub4         k;
	 ub4         ColumnCount;
	 
	 
     memset(ptr->errbuf,0,512);
     printf("sql=%s\n",sql);
     printf("sql len=%d\n",sqllen);
     
     
     OCIStmtPrepare(ptr->stmthp,ptr->errhp,sql,sqllen,OCI_NTV_SYNTAX,OCI_DEFAULT);
     //OCIErrorGet( ptr->errhp, ( ub4 ) 1, ( text * ) 0,
     //              &ptr->errcode, ptr->errbuf, ( ub4 ) sizeof( ptr->errbuf ),
     //              OCI_HTYPE_ERROR );
     //printf("msg:## OCIStmtPrepare %d %s\n", ptr->status, ptr->errbuf);
     
     
     OCIAttrGet(ptr->stmthp, OCI_HTYPE_STMT, (dvoid *)&stmttype,(ub4 *)NULL, OCI_ATTR_STMT_TYPE,ptr->errhp);
     //printf("SQL=%s\n",sql);     
     
     if(stmttype==OCI_STMT_SELECT)stmttype=0;
     else stmttype=1;
     
     
     
     if(ptr->commitmode==0)commitmode=OCI_COMMIT_ON_SUCCESS;
     else commitmode=OCI_DEFAULT;

     status=OCIStmtExecute(ptr->svchp, ptr->stmthp, ptr->errhp, (ub4) stmttype, (ub4) 0, 
                    (OCISnapshot *) NULL, (OCISnapshot *) NULL, commitmode );
     OCIErrorGet( ptr->errhp, ( ub4 ) 1, ( text * ) 0,
                   &ptr->errcode, ptr->errbuf, ( ub4 ) sizeof( ptr->errbuf ),
                   OCI_HTYPE_ERROR );
     
     
	 if(ptr->errcode==1012 ||( ptr->errcode==3114)||   (ptr->errcode==041 )   ){//not login or session has been killed
		printf("msg:## ptr->errcode = %d ptr->errbuf=%s \n", ptr->errcode,ptr->errbuf);
		printf("relogin------ \r");
		
		
		
		/*
		if(oci_login(ptr,host,user,pwd)==-1){
			printf("login fail \n");
		}*/
		
		return -99;
	 }
	 
	 
     //return -1;
     
     if(status<0){
     	return -1;
     }
     num_cols=0;
     OCIAttrGet(ptr->stmthp, OCI_HTYPE_STMT, &num_cols, 0, OCI_ATTR_PARAM_COUNT, ptr->errhp);                    
     
     for(i=0;i<num_cols;i++){
	    OCIDefineByPos(ptr->stmthp,&ptr->defnp,ptr->errhp,i+1,ptr->data[i],sizeof(ptr->data[i]),SQLT_STR,0,0,0,OCI_DEFAULT);
     }
     
     
     OCIAttrGet(ptr->stmthp, (ub4)OCI_HTYPE_STMT, (dvoid*)&ColumnCount, (ub4 *) 0, (ub4)OCI_ATTR_PARAM_COUNT, ptr->errhp); 	  	 
	 for(k=1; k<=ColumnCount ; k ++){
	 	 OCIParamGet(ptr->stmthp, OCI_HTYPE_STMT, ptr->errhp, (dvoid **)&param, k);
	 	 OCIAttrGet((dvoid*)param, OCI_DTYPE_PARAM, (dvoid**)&ColumnName,(ub4 *)&ColumnNameLength, OCI_ATTR_NAME, ptr->errhp);
	     sprintf(ptr->fieldname[k-1],"%s",ColumnName);
	 }
     
     
     return num_cols;

}


int fetch_data(pool_oci *ptr){
  int status;
  ub1   in_out = 0;
  sb2   indptr = 0;
  sb4 rcode;
ub1   piece = OCI_FIRST_PIECE;

  ub4 	hdltype = OCI_HTYPE_DEFINE, iter = 0, idx = 0;
   dvoid *hdlptr = (dvoid *) 0, *hdlptr_tmp = 0;


     if((ptr->status = OCIStmtFetch2 ( ptr->stmthp, ptr->errhp, 1,0, OCI_FETCH_NEXT, OCI_DEFAULT ) ) != OCI_NO_DATA){ 
           if(OCI_NEED_DATA==ptr->status){
           status =OCIStmtGetPieceInfo(ptr->stmthp, ptr->errhp, &hdlptr, &hdltype, &in_out, &iter, &idx, &piece);
	   }
           return 1;
     }
     else{
           return -1;
     }
}



void oci_logout(pool_oci *ptr){
    OCILogoff(ptr->svchp,ptr->errhp);
    OCIHandleFree(ptr->envhp,OCI_HTYPE_ENV);
    OCITerminate(OCI_DEFAULT);
}

