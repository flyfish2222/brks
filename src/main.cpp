#include "DispatchMsgService.h"
#include "interface.h"
#include "Logger.h"
#include "sqlconnection.h"
#include "BusProcessor.h"
#include "configdef.h"

#include <functional>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common/luaconfig.h"

#define OPTERRCOLON (1)
#define OPTERRNF (2)
#define OPTERRARG (3)
#define DEFAULT_BIN_NAME "brks"

int optreset = 0;
int optind = 1;
int opterr = 1;
int optopt;

static void create_multi_process(int process_count)
{
	for (int i = 0; i < process_count; ++i)
	{
		switch (fork())
		{
		case -1:
			LOG_INFO("fork child task failed.");
			exit(EXIT_FAILURE);
			break;
		case 0:
			LOG_INFO("brks start successful! pid = %d", getpid());
			return;
		default:
			break;
		}
	}
}

static bool make_daemon()
{
	pid_t pid = fork();
	if (-1 == pid)
		return false;

	if (0 != pid)
		_exit(EXIT_SUCCESS);

	if (-1 == setsid())
		return false;

	// 消除会话进程获取终端的可能
	pid = fork();
	if (-1 == pid)
		return false;

	if (0 != pid)
		_exit(EXIT_SUCCESS);

	// 避免工作目录所在设备被卸载
	if (-1 == chroot("/"))
		return false;

	umask(0766);

	int fd = open("/dev/null", O_RDWR);
	if (-1 == fd)
		return false;

	if (-1 == dup2(fd, STDIN_FILENO)
		|| -1 == dup2(fd, STDOUT_FILENO)
		|| -1 == dup2(fd, STDERR_FILENO))
	{
		close(fd);
		return false;
	}

	if (-1 == close(fd))
		return false;

	return true;
}

static void usage(const std::string& program)
{
	printf("usage: %s [ -d | -h ] <-c brks.lua>\n", program.c_str());
	printf("\t-h: help list \n");
	printf("\t-d: run in the daemon\n");
	printf("\t-c brks.lua: json config file\n");
}

static void do_options(LuaConfig& luaConfig, int argc, char* argv[])
{
	if (argc <= 1)
	{
		usage(argv[0]);
		exit(EXIT_SUCCESS);
	}

	char ch;
	while (-1 != (ch = getopt(argc, argv, "dhc:")))
	{
		switch (ch)
		{
		case 'h':
			usage(argv[0]);
			exit(EXIT_SUCCESS);
			break;
		case 'd':
			if (!make_daemon())
			{
				perror("daemon");
				exit(EXIT_FAILURE);
			}
			break;
		case 'c':
			if (!luaConfig.DoFile(optarg))
				exit(EXIT_FAILURE);
			break;
		default:
			break;
		}
	}
}

static void signal_child_callback(int signum)
{
	auto tempErrno = errno;

	int stat, exitStatus;
	pid_t pid;
	while (true)
	{
		if ((pid = waitpid(-1, &stat, WNOHANG)) <= 1)
			break;

		exitStatus = WEXITSTATUS(stat);
		LOG_INFO("wait process return with: pid=%d, status=%d, exitStatus=%d.", pid, stat, exitStatus);
	}

	errno = tempErrno;
}

int main(int argc, char** argv)
{
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, signal_child_callback);

	LuaConfig luaConfig;
	do_options(luaConfig, argc, argv);

	if (!luaConfig)
		exit(EXIT_FAILURE);

	const st_env_config& config = luaConfig.GetConfig();

	if (!Logger::instance()->init(config.log_conf.c_str()))
	{
		printf("init log module with file path = %s failed.\n", config.log_conf.c_str());
		return -2;
	}

	std::shared_ptr<DispatchMsgService> dms(new DispatchMsgService);
	dms->open();

	std::shared_ptr<MysqlConnection> mysqlconn(new MysqlConnection);
	//mysqlconn->Init("127.0.0.1", 3306, "root", "123456", "dongnaobike");
	mysqlconn->Init(config.db_name.c_str(),
		config.db_port,
		config.db_user.c_str(),
		config.db_pwd.c_str(),
		config.db_name.c_str());

	BusinessProcessor processor(dms, mysqlconn);
	processor.init();

	std::function< iEvent* (const iEvent*)> fun = std::bind(&DispatchMsgService::process, dms.get(), std::placeholders::_1);

	// create server socket and set to non block
	Interface intf(fun);
	int server_socket = intf.create_and_bind_socket(config.svr_port);
	intf.set_socket_non_block(server_socket);

	if (!intf.add_server_socket(server_socket))
	{
		LOG_ERROR("cannot start child process!");
		exit(-1); // if cannot start child process donnot restart server.
	}

	create_multi_process(config.process_count - 1);

	intf.run();
	LOG_ERROR("child process exit");

	for (;;);

	return 0;
}


