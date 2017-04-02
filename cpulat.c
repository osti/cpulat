/* http://en.community.dell.com/cfs-file/__key/telligent-evolution-components-attachments/13-4491-00-00-20-22-77-64/Controlling_5F00_Processor_5F00_C_2D00_State_5F00_Usage_5F00_in_5F00_Linux_5F00_v1.1_5F00_Nov2013.pdf */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int back = 0;
const char* pidfile  = "/var/run/cpulat.pid";
const char* confname = "/etc/cpulat.conf";

struct cpulat {
	char* acname;
	char* latname;
	char* tmpname;
	int sleeptime;
	int latency;
};
struct cpulat conf;
int config(const char* confname);

int main(int argc, const char* argv[])
{
	int ret = 0;
	pid_t pid;

	if (argc != 1 && argc != 2)
	{
		fprintf(stderr, "usage: %s [-d|--daemon]\n", argv[0]);
		ret = -1;
		goto cleanup;
	}
	if (argc == 2 && (strcmp(argv[1], "-d")==0 || strcmp(argv[1], "--daemon")==0))
	{
		pid = fork();
		if (pid<0)
		{
			fprintf(stderr, "cannot fork: %s\n", strerror(errno));
			ret = -1;
			goto cleanup;
		}
		if (pid>0)
		{
			FILE* f = NULL;
			f = fopen(pidfile, "w");
			if (f) fprintf(f, "%d\n", pid);
			if (f) fclose(f); f = NULL;
			fprintf(stdout, "forking into background, pid %d\n", pid);
			goto cleanup;
		}
		if (pid==0) back=1;
	}

	conf.latname = strdup("/dev/cpu_dma_latency");
	conf.sleeptime = 5;
	conf.latency   = 250;

	while (1)
	{
		while (config(confname)!=0) sleep(1);

		/* check if we are running on AC */
		int ac, tmp;
		FILE* f = NULL;
		int fd  = -1;

		if (conf.acname && strlen(conf.acname))
		{
			f = fopen(conf.acname, "r");
			if (!f || fscanf(f, "%d", &ac)!=1 || ac!=1) goto wait;
		}

		fd = open(conf.latname, O_WRONLY);
		if (fd < 0) goto wait;
		if (write(fd, &conf.latency, sizeof(conf.latency)) != sizeof(conf.latency)) goto wait;

wait:
		sleep(conf.sleeptime);
		if (f)    fclose(f); f  = NULL;

		f = fopen(conf.tmpname, "r");
		if (f && fscanf(f, "%d", &tmp)==1 && back==0)
		{
			fprintf(stdout, "\r%.1f℃", 0.001*tmp);
			fflush(stdout);
		}

		if (f)    fclose(f); f  = NULL;
		if (fd>0) close(fd); fd = -1;
	}
	
cleanup:
	return ret;
}

int config(const char* confname)
{
	int ret = 0;
	FILE* f = fopen(confname, "r");
	if (f == NULL) goto cleanup;

	if (conf.acname)  free(conf.acname);  conf.acname  = NULL;
	if (conf.tmpname) free(conf.tmpname); conf.tmpname = NULL;

	while (!feof(f))
	{
		char line[256];
		char buf[256];
		int value;
		memset(line, 0, sizeof(line));
		memset(buf, 0, sizeof(buf));
		if (fgets(line, sizeof(line)-1, f) == NULL) continue;
		if (line[0] == '#') continue;

		if (sscanf(line, "LATENCY_FILE %64s\n",     buf)==1) { free(conf.latname); conf.latname = strdup(buf); }
		if (sscanf(line, "TEMPERATURE_FILE %64s\n", buf)==1) { free(conf.tmpname); conf.tmpname = strdup(buf); }
		if (sscanf(line, "CHECK_AC_FILE %64s\n",    buf)==1) { free(conf.acname ); conf.acname  = strdup(buf); }
		if (sscanf(line, "LATENCY %d\n",         &value)==1) { conf.latency   = value; }
		if (sscanf(line, "SLEEP %d\n",           &value)==1) { conf.sleeptime = value; }
	}

cleanup:
	if (f) fclose(f); f = NULL;
	return ret;
}
