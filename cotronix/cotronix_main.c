#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#define		MAX_DATA	48

enum ERROR
{
	error_free,
	error_load_on_off_pattern,
	error_wrong_pattern,
	error_open_On_off_pattern,
	
	
};

unsigned char error_code;

int local_half_hour_count, prev_local_half_hour_count;
char buffer[30];
struct tm* tm_info;

char on_off_data[MAX_DATA+2];
char sel_opt[4];
bool on_off_pattern_loaded = false;


void file_input();

void timer_callback(int signum)
{
	time_t now = time(NULL);
	tm_info = localtime(&now);
//    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
//    puts(buffer);
    
    local_half_hour_count = tm_info->tm_hour;
    local_half_hour_count *=2;
    local_half_hour_count += tm_info->tm_min / 30;
    
    if (local_half_hour_count != prev_local_half_hour_count)
    {
		prev_local_half_hour_count = local_half_hour_count;
		
		strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
		puts(buffer);
				
		if (on_off_data[local_half_hour_count] == '1')
			printf("ON for %dth of 30 mins\n\n", local_half_hour_count);
		else
			printf("OFF for %dth of 30 mins\n\n", local_half_hour_count);
	}
 	

}


int main()
{
	int ii;
	
	for (ii=0; ii<MAX_DATA; ii++)
		on_off_data[ii] = '0';
		
	prev_local_half_hour_count = -1;
	local_half_hour_count = 0;
	on_off_pattern_loaded = false;
	error_code = error_free;
	
	struct itimerval new_timer;
	struct itimerval old_timer;

	new_timer.it_value.tv_sec = 0;
	new_timer.it_value.tv_usec = 0;
	new_timer.it_interval.tv_sec = 0;
	new_timer.it_interval.tv_usec = 0; 

	setitimer(ITIMER_REAL, &new_timer, &old_timer);
	signal(SIGALRM, timer_callback);
	sleep(1);
	
	while (1)
	{
		printf("\n\nPlease select the following options:\n\n");
		printf("1. Input data file.\n");
		printf("2. Stop the process.\n");
		printf("3. Start the process.\n");
		printf("4. Check the status. \n");
		printf("e. Exit the process.\n\n");
		scanf("%s",sel_opt);
		
		if (sel_opt[0] == '1')		// load a on-off-pattern file
		{
			file_input();
			if (on_off_pattern_loaded == true)
			{
				printf("The on-off-pattern is: %s\n", on_off_data);
			}
			else
			{
				printf("There is no on-off-pattern loaded!!\n");
			}
				
		}
		else if (sel_opt[0] == '2')		// stop the process
		{
			printf("The process is stoped!!\n");
			new_timer.it_value.tv_sec = 0;
			new_timer.it_value.tv_usec = 0;
			new_timer.it_interval.tv_sec = 0;
			new_timer.it_interval.tv_usec = 0; 

			setitimer(ITIMER_REAL, &new_timer, &old_timer);
			
			if (on_off_pattern_loaded == true)
			{
				printf("The on-off-pattern is: %s\n", on_off_data);
			}
			else
			{
				printf("There is no on-off-pattern loaded!!\n");
			}			
			
		}
		else if (sel_opt[0] == '3')		// start the process
		{
			if (on_off_pattern_loaded == true)
			{
				printf("Process starts!!\n");
				prev_local_half_hour_count = -1;
				local_half_hour_count = 0;
		
				new_timer.it_value.tv_sec = 0;
				new_timer.it_value.tv_usec = 500;
				new_timer.it_interval.tv_sec = 1;
				new_timer.it_interval.tv_usec = 0; 

				setitimer(ITIMER_REAL, &new_timer, &old_timer);
				sleep(1);
			}
			else
			{
				printf("There is no on-off-pattern loaded!!\n");
			}
			
		}
		else if (sel_opt[0] == '4')
		{
			printf("The on-off-pattern is: %s\n", on_off_data);
		}
		else if (sel_opt[0] == 'e')
		{
			new_timer.it_value.tv_sec = 0;
			new_timer.it_value.tv_usec = 0;
			new_timer.it_interval.tv_sec = 0;
			new_timer.it_interval.tv_usec = 0; 

			setitimer(ITIMER_REAL, &new_timer, &old_timer);
			printf("The process is stoped and exited.\n");
			sleep(1);
			
			break;
			
		}
		else
		{
			printf("invalid input\n");
		}

	}

	return 0;
}

void file_input()
{
	FILE *fptr;
	int ii, num;
	char file_name[32], temp_char;
	
	on_off_pattern_loaded = false;
	
	printf("Please input a filename of contenting the on/off data\r\n");
	scanf("%32s", file_name);
		
	fptr = fopen(file_name, "r");
		
	if (fptr == NULL)
	{
		printf("The file is not existing\r\n");
		error_code = error_open_On_off_pattern;
		return;
	}
		
	fseek(fptr, 0, SEEK_SET);
		
	for (ii=0; ii<MAX_DATA; ii++)
	{
		num = fread(&temp_char, 1, 1, fptr);
		
		if (num == 0)
		{
			printf("There is some issues with reading the file\r\n");
			error_code = error_load_on_off_pattern;
			fclose(fptr);
			return;
		}
		else
		{
			if ((temp_char == '1') || (temp_char == '0'))
			{
				on_off_data[ii] = temp_char;				
			}
			else
			{
				printf("The data in the file is not in the right format.\r\n");
				error_code = error_wrong_pattern;
				fclose(fptr);
				return;
			}
			
		}
	}
	
	fclose(fptr);
	
	on_off_pattern_loaded = true;
		
	printf("On-off-pattern Loaded completed\r\n");
	
}
