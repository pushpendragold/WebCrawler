all : main

main : WebSpider.o main.o LogTrace.o MySQL.o PostgreSQL.o JSONParser.o
	g++ -g WebSpider.o LogTrace.o MySQL.o PostgreSQL.o JSONParser.o main.o -o main -l curl -l mysqlclient -lpqxx -lpq

WebSpider.o: WebSpider.cpp
	g++ -g -c WebSpider.cpp -l curl -l mysqlclient

LogTrace.o: LogTrace.cpp
	g++ -g -c LogTrace.cpp -l curl -l mysqlclient

MySQL.o: MySQL.cpp
	g++ -g -c MySQL.cpp -l curl -l mysqlclient

PostgreSQL.o: PostgreSQL.cpp
	g++ -g -c PostgreSQL.cpp -l curl -lpqxx -lpq

JSONParser.o: JSONParser.cpp
	g++ -g -c JSONParser.cpp -l curl -lpqxx -lpp

main.o : main.cpp
	g++ -g -c main.cpp -l curl -l mysqlclient -lpqxx -lpq

clean :
	rm -rf *.o *~* main *core* visited* trace dump* test1/*
