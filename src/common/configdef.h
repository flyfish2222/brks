#ifndef BRKS_COMMON_CONFIGDEF_H_
#define BRKS_COMMON_CONFIGDEF_H_

#include <sstream>

typedef struct st_env_config
{
	std::string db_ip;
	int db_port;
	std::string db_user;
	std::string db_pwd;
	std::string db_name;
	std::string log_conf;
	int process_count;

	//·þÎñµÄÅäÖÃ
	unsigned short svr_port;

	st_env_config()
	{
	};

	st_env_config(const std::string& db_ip, int db_port,
		const std::string& db_user, const std::string& db_pwd,
		const std::string& db_name, unsigned short svr_port,
		std::string& log_conf, int process_count)
	{
		this->db_ip = db_ip;
		this->db_port = db_port;
		this->db_user = db_user;
		this->db_pwd = db_pwd;
		this->db_name = db_name;
		this->svr_port = svr_port;
		this->log_conf = log_conf;
		this->process_count = process_count;
	};

	st_env_config& operator =(const st_env_config& config)
	{
		if (this != &config)
		{
			this->db_ip = config.db_ip;
			this->db_port = config.db_port;
			this->db_user = config.db_user;
			this->db_pwd = config.db_pwd;
			this->db_name = config.db_name;
			this->svr_port = config.svr_port;
			this->log_conf = config.log_conf;
			this->process_count = config.process_count;
		}
		return *this;
	}

	std::ostream& dump(std::ostream& out) const
	{
		std::ostringstream oss;
		oss << "{"
			<< "db_ip:" << this->db_ip
			<< ", db_port:" << this->db_port
			<< ", db_user:" << this->db_user
			<< ", db_pwd:" << this->db_pwd
			<< ", db_name:" << this->db_name
			<< ", svr_port:" << this->svr_port
			<< ", log_conf:" << this->log_conf
			<< ", process_count:" << this->process_count
			<< "}";

		out << oss.str() << std::endl;

		return out;
	}
}_st_env_config;


#endif

