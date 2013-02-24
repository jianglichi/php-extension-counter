#TARGET = test.e
######### ENV ##########
CC = gcc
OBJECTS = test.o
############### INCLUDES ###################
#/usr/include/oracle/10.2.0.3/client/occi.h
#/usr/lib/oracle/10.2.0.3/client/lib
#/usr/lib/oracle/10.2.0.3/client/lib/orai18n.jar
ORACLE_HOME = /usr/include/oracle/10.2.0.3/client
#ORACLE_INCLUDES = -I$(ORACLE_HOME)/rdbms/public -I$(ORACLE_HOME)/rdbms/demo
ORACLE_INCLUDES = -I/usr/include/oracle/10.2.0.3/client
INCLUDES += $(ORACLE_INCLUDES)
#ORACLE_LIBS = -L$(ORACLE_HOME)/lib -O3 -lnsl -lpthread -lclntsh -lclient9 -lsqlplus -lcore9
#ORACLE_LIBS = -L/usr/lib/oracle/10.2.0.3/client/lib -O3 -lnsl -lpthread -lclntsh -lclient9 -lsqlplus -lcore9
ORACLE_LIBS = -L/usr/lib/oracle/10.2.0.3/client/lib -O3 -lnsl -lpthread -lclntsh -locci -lsqlplus -lociei -lheteroxa10 -lnnz10 -lsqlplusic
ORACLE_LIBS .= -lp_fun -lrt -lkstat
###########################################


all:
	@echo;echo "============================== [$@] =============================="
	$(CC) -o mypool mypool.c $(INCLUDES)  $(ORACLE_LIBS)
