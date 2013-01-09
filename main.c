#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <error.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>
//#include <popt.h>

#define MAX_DIR 512
#define BUFSIZE 4096

typedef unsigned long int LL;

static LL countAll	= 0;
static LL sizeAll		= 0;
static LL summAll		= 0;

static flagTime = 0;

static flagLink = 0;

//count tree in recur function
static count_tree = 0;

/*
static int showTime = 0;
struct poptOption optionsTable[] = 
{
	{"show time", 't', POPT_ARG_NONE, &showTime, 0,
	 "Отображает время затраченное на подсчет контрольной суммы", NULL},
POPT_AUTOHELP
 {NULL, '\0', POPT_ARG_NONE, NULL, 0, NULL, NULL}
};
*/

void handler_sig(int numsig)
{
	printf("count = %u\n", countAll);
	printf("current size = %u\n", sizeAll);
}

void check_file(char *name, size_t len_f)
{
	int in_fd = 0;
	//size_t len_f = st.st_size;
	
	if(len_f > 0)
	{
		in_fd = open(name, O_RDONLY);
		if(in_fd != -1)
		{
			int len1 = 0;
			int jjj = 0;
			unsigned char temp2[BUFSIZE];
			while( (len1 = read(in_fd, temp2, BUFSIZE)) > 0 )
			{
				int iii;
				for(iii = 0; iii < len1; iii++)
				{
					summAll += (temp2[iii] * (jjj + iii + 1));
				}
				jjj += len1;
			}
			close(in_fd);
		}
		else
		{
			perror("open file");
		}
	}
	sizeAll += len_f;
	countAll++;
}

void check_dir(char *dir)
{
	int i;
	struct stat st;
	int count = 0;
	struct dirent **namelist = 0;
	struct dirent *d;
	char *dir1 = 0;
	int len = 0;


	if(count_tree == 0)
	{
		int res;
//check i
		if(flagLink)
		{
			res = stat(dir, &st);
		}
		else
		{
			res = lstat(dir, &st);
		}
		if(res == 0)
		{
			if ( (st.st_mode & S_IFMT) == S_IFREG )
			{
				//printf("It is a file\n");
				check_file(dir, st.st_size);
				return;
			}
			else
			{
				//printf("It is a dir\n");
			}
		}
	}

	//printf("tree %d\n", count_tree);
	count_tree++;


	if(dir && ((len = strlen(dir)) > 0) )
	{
		dir1 = (char*)malloc(len + 1);
		strcpy(dir1, dir);
	}
	else
	{
		printf("Error. Check_dir wrong dir\n");
		return;
	}


	
	if(dir1[len-1] != '/')
	{
		free(dir1);
		dir1 = (char*)malloc(len + 2);
		strcpy(dir1, dir);
		strcat(dir1, "/");
	}

	count = scandir(dir, &namelist, 0, 0);

	if(count < 0)
	{
		//printf("Error. read dir %s\n", dir);
	}
	else
	{
		for(i = 2; i<count; i++)
		{
			char name[1024];
			int res;
			strcpy(name, dir1);
			strcat(name, namelist[i]->d_name);

			if(flagLink)
			{
				res = stat(name, &st);
			}
			else
			{
				res = lstat(name, &st);
			}

			if(res == 0)
			{
				if ( (st.st_mode & S_IFMT) != S_IFREG )
				{
					check_dir(name);
				}
				else
				{
					check_file(name, st.st_size);
				}
			}
			else
			{
				//printf("Error. read file %s\n", name);
			}
		}
		free(namelist);
	}
	free(dir1);
	count_tree--;
}


int main(int argc, char **argv)
{

	struct timeval tv;
	struct timeval tz;
	double T1 = 0.0;

	char *strFormat = "check_summ.e [OPTION...] PATH\n-t show used time\n-l use links as files\n";

	char s_dir[MAX_DIR];
	
	if(argc < 2)
	{
		printf(strFormat);
		return -1;
	}
	else
	{

		while(1)
		{
			int c;

			static struct option long_options[] = 
			{
				{"t", 0, 0, 0},
				{"l", 0, 0, 0}
			};

			int option_index = 0;

			c = getopt_long(argc, argv, "t l", long_options, &option_index);

			if(c == -1)
			{
				break;
			}
			
			switch(c)
			{
				case 't':
					flagTime = 1;
					//printf("set option show time\n");
				break;

				case 'l':
					flagLink = 1;
					//printf("set option link as file\n");
				break;
			}
		}

		if(argv[optind])
		{
			strcpy(s_dir, argv[optind]);
		}
		else
		{
			printf("Error. PATH is NULL!!!\n");
			printf(strFormat);
			return -1;
		}
	}

	signal(SIGINT, handler_sig);

	if(flagTime)
	{
		gettimeofday(&tv, &tz);
		T1 = (double)(tv.tv_sec + tv.tv_usec*0.000001);
	}

	check_dir(s_dir);

	printf("%u\n", summAll);

	if(flagTime)
	{
		gettimeofday(&tv, &tz);
		printf("time %.3f\n", (double)(tv.tv_sec + tv.tv_usec*0.000001) - T1);	
	}

	return 0;
}