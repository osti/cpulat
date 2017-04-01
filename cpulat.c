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
const char* pidfile = "/var/run/cpulat.pid";

int main(int argc, const char* argv[])
{
	int ret = 0;
	pid_t pid;
	int lat = 250;

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

	while (1)
	{
		/* check if we are running on AC */
		const char* acname  = "/sys/class/power_supply/AC/online";
		const char* latname = "/dev/cpu_dma_latency";
		const char* tmpname = "/sys/class/hwmon/hwmon1/temp1_input";
		int ac, tmp;
		FILE* f = NULL;
		int fd  = -1;

		f = fopen(acname, "r");
		if (!f || fscanf(f, "%d", &ac)!=1 || ac!=1) goto wait;

		fd = open(latname, O_WRONLY);
		if (fd < 0) goto wait;
		if (write(fd, &lat, sizeof(lat)) != sizeof(lat)) goto wait;

wait:
		sleep(5);
		if (f)    fclose(f); f  = NULL;

		f = fopen(tmpname, "r");
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

