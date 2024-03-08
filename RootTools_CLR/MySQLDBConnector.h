#pragma once
#include <stdio.h>
#include <string>
#include "mariadb/mysql.h"

#include <vcclr.h> // for PtrToStringChars 
#include <stdio.h> // for wprintf
#include <msclr\marshal_cppstd.h>

#pragma warning(disable: 4267)

namespace RootTools_CLR
{
	public ref class MySQLDBConnector
	{
	public:
		unsigned int OpenDatabase()
		{
			MYSQL* mysql;
			const char* query;
			unsigned int error_code;
			mysql = mysql_init(NULL);

			//여기서 서버에 업로드..인데 이러면 객체로 만들 필요도 없는데?
			if (!mysql_real_connect(mysql, "localhost", "root", "`ati5344", "Inspections", 0, "/tmp/mysql.sock", 0))
			{
				error_code = mysql_errno(mysql);
				//1044 권한없는찐따
				//1049 DB가 업서요
				//1064 쿼리문이 잘못됨
				//1146 Table이 없음
				//1136 컬럼개수가 일치하지않음
				if (error_code == 1049)
				{
					if (!mysql_real_connect(mysql, "localhost", "root", "`ati5344", NULL, 0, "/tmp/mysql.sock", 0))
					{
						error_code = mysql_errno(mysql);
						return error_code;
					}
					else 
					{
						maxExpand(mysql);

						query = "CREATE DATABASE Inspections;";
						if (mysql_real_query(mysql, query, strlen(query)))
						{
							error_code = mysql_errno(mysql);
							mysqlError(mysql, error_code);
							return error_code;
						}
						else 
						{
							mysql_close(mysql);
							return 0;
						}
					}
				}
				else 
				{
					mysqlError(mysql, error_code);
					return error_code;
				}
			}
			else 
			{
				mysql_close(mysql);
				return 0;
			}

			return 0;
		}
		unsigned int RunQuery(System::String^ inputQuery)
		{
			MYSQL* mysql;
			unsigned int error_code;
			mysql = mysql_init(NULL);
			int sockIdx = std::rand();

			if (!mysql_real_connect(mysql, "localhost", "root", "`ati5344", "Inspections", 0, "/tmp/mysql.sock", 0))
			{
				error_code = mysql_errno(mysql);
				return error_code;
			}

			msclr::interop::marshal_context context;
			std::string standardString = context.marshal_as<std::string>(inputQuery);
			if (mysql_real_query(mysql, standardString.c_str(), strlen(standardString.c_str())))
			{
				error_code = mysql_errno(mysql);
				mysqlError(mysql, error_code);
				return error_code;
			}
			else
			{
				mysql_close(mysql);
				return 0;
			}


			return 0;
		}
	private:
		void mysqlError(MYSQL* mysql, unsigned int errCode)
		{
			mysql_close(mysql);
			//exit(-1);
		}
		void maxExpand(MYSQL* mysql)
		{
			const char* query = "SET GLOBAL max_connections = 10000;";
			if (mysql_real_query(mysql, query, strlen(query)))
			{
				unsigned int error_code = mysql_errno(mysql);
				mysqlError(mysql, error_code);
			}
		}
	};
}