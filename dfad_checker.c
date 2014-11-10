#include "helper.h"

long array[10];			//push the fd onto this stack

long inode_nums[10];		//Store the inode numbers in this stack
int inode_index = 0;

char open_files[10][100];

// NOT USING FUNCTION getdata (pid_t child, long addr, char *str, int len) ANY MORE :P
// available in helper.h

// Function to check mode : int mode_check_1 (char c) in helper.h

/*STACK FUNCTIONS START HERE!!!
void push (int x, int* array) 
int pop(int* array,int i) 
int check_array(long x, int* array) 
void list_array(int* array) 
STACK FUCNTIONS END HERE !!*/

//print_file_mode (int file_access_mode, int counter, FILE *fp) 

int main(int argc, char** argv) {	

pid_t child;
long orig_eax, eax;
long reg_val[3];
int status ;
int insyscall_1 =0;
int insyscall_2 =0;
int insyscall_3 =0;
int insyscall_4 =0;
int insyscall_5 =0;

char arguments[argc-1][100];				// Inital arguments are pushed into this 
char dir_path[10][100];

int counter = 0;
int file_access_mode;
child = fork ();
FILE *fp = fopen ("temp.txt","w");			//Reading the Data flow properties to file
int i;
for(i=0;i<10;i++) {					//Initializing the array[] to -1
	array[i]= -1;
}
if (child == -1) 
	printf ("Error in fork");

if (child == 0) {
	// In the child process
	ptrace (PTRACE_TRACEME,0,NULL,NULL);
//execl("/home/cdac/attack_sleep","attack_sleep","/home/cdac/fic", "cdac", NULL);
//execl("/home/cdac/attack","attack","a.txt", "root::1:99999:::::", NULL);
	execv("/home/cdac/Documents/ptrace/file_write", argv);//"file_write","a.txt", "b.txt", NULL);
//	execv("/bin/ls", argv);
//execl ("/home/cdac/Documents/ptrace/client_server_model/client", "client", "localhost", "20000", NULL);
}

else {	
	printf("In parent process\n");
	int i, var = 0, file_counter = 0;
	for(i=1;i < argc;i++) {				// Initially used to push the arguments in the open_files now in argumets[10][100] 
		if (var < 10) {
			strcpy(arguments[var],argv[i]);
			var++;
		}
	}
	
//	char* working_dir = (char *)get_current_dir_name();
//	printf("%s", working_dir);

	while (1) {
		int syscall;
                struct user_regs_struct u_in;			
													//Loop to catch each of the system calls
		wait (&status);										//wait for the child process to finish
		if (WIFEXITED(status))									//The child process exited
			break;
		syscall = ptrace (PTRACE_PEEKUSER, child, 4 * ORIG_EAX, NULL);
		
		if (syscall == SYS_open) {
			struct stat inode_no;								 
			if (insyscall_1 == 0) {
				counter++;
				insyscall_1 = 1;
				ptrace(PTRACE_GETREGS,child, 0, &u_in);		
				reg_val[1] = ptrace (PTRACE_PEEKUSER, child, 4 * EBX, NULL);
				reg_val[2] = ptrace (PTRACE_PEEKUSER, child, 4 * ECX, NULL);
				reg_val[3] = ptrace (PTRACE_PEEKUSER, child, 4 * EDX, NULL);		//not used in access syscall
				printf ("System call is : %d\t\t/open\n", syscall);			//Writing a work around for a erroneous read!!!!
				// We call the function as read_string
				//fprintf(stderr, "ebx :%08lx ecx:  %08lx edx:%08lx\n", reg_val[1], reg_val[2], reg_val[3]);
				//filepath = (char *)calloc (50 , sizeof(char));
				//getdata (child, reg_val[1], filepath,50);*/
				char buff[4];
				sprintf(buff, "%lo", reg_val[2]);
				if (strlen (buff) == 4) {
					file_access_mode = mode_check_1(buff[3]);

				}
				else if (strlen (buff) == 3)	{	
					file_access_mode = mode_check_1 (buff[2]);

				}
				// There are no rules for the first bit i.e, for strlen(buff) == 2
				else if ( strlen (buff) == 1) {
					file_access_mode = mode_check_1 (buff[0]);

				}
				char *filepath = read_string(child, reg_val[1]);
				
				//Getting the inode number from filename

				lstat (filepath, &inode_no);						// system call is for sys_access
				printf("The files inode number is : %ld\n",(long)inode_no.st_ino);
	//			printf("%ld",(long int)inode_no.st_mode);
					
				
				// (tmp*) creating a stack and pushing;
				
				// We check the file modes only for files which are not starting with /etc and /lib
				char *sys_file1 = "/etc/";
				char *sys_file2 = "/lib/";
				if (strncmp(filepath,sys_file1,5)  == 0 || strncmp(filepath,sys_file2,5)  == 0 )
					insyscall_1 = 2; 					// this makes the else part satisfy the IF condition
				
				
				printf ("The location of the file is: %s\n", filepath);
				//open_files[file_counter][100] = read_string(child, reg_val[1]);
				if  (file_counter < 10) {
					strcpy(open_files[file_counter],filepath);
					file_counter++;
				}
				
				// Function call to check if the files is in already	
				//Checking if the Opened file is an argument passed
				int i;
				for (i =0; i < argc-2;i++) {
					if (strcmp (filepath, arguments[i]) == 0) {
						fprintf(fp, "f%d equals I\n",counter);
						if (model_check () == -1 ) {
							exit(0);
						} 
					}		
				}
			}
				
			else {
				eax = ptrace (PTRACE_PEEKUSER, child, 4 * EAX, NULL);
				if (insyscall_1 != 2) {
					if (inode_index < 10 )  {
						inode_nums [inode_index] = inode_no.st_ino;
						push (inode_nums [inode_index], array);		//list_array(array);//jst a check
						print_file_mode(file_access_mode, counter, fp);
						inode_index ++;
					}
				}
				printf ("Return value of the SYS_open is %ld\n\n\n\n",eax);
				insyscall_1 = 0;
				}
		}
	//If the system call is SYS_Read
		if (syscall == SYS_read) {
		struct stat sb;
		int iterator;
		long reg_val_temp[3];
		if (insyscall_2 == 0) {
			char rule[4096];
			counter++;
			insyscall_2 = 1;
			ptrace (PTRACE_GETREGS, child, 0, &u_in);
			reg_val_temp[1] = ptrace (PTRACE_PEEKUSER, child, 4 * EBX, NULL);
			reg_val_temp[2] = ptrace (PTRACE_PEEKUSER, child, 4 * ECX, NULL);
			reg_val_temp[3] = ptrace (PTRACE_PEEKUSER, child, 4 * EDX, NULL);
			printf ("System call is: %d\t\t/read\n", syscall);
	// Not able to print the buffer location :(
	//fprintf(stderr, "poninter to buf is: %ld\n", reg_val_temp[2]);
			fprintf(stderr, "File Descriptor is: %ld\nNo of bytes to read: %ld\n ", reg_val_temp[1], reg_val_temp[3]);
	//int temp = check_array(reg_val_temp[1], array); initially I checked with File Descriptors , Now using inode numbers to check
			fstat (ptrace (PTRACE_PEEKUSER, child, 4 * EBX, NULL), &sb);				// Getting the inode number from FD
			int temp = -1;
		
			for (iterator = 0; iterator < inode_index; iterator++) {
				printf ("%ld\n ", inode_nums[iterator]);
				if (inode_nums[iterator] == sb.st_ino ) {
						temp = 1;
					break;
				}
			}
		if (temp != -1) {
				fprintf (fp, "FD%d equals FD%d\n", counter, temp);
				sprintf (rule, "FD%d equals FD%d\n", counter, temp);
				if (model_check (line_number, rule) == -1 ) {
					exit(0);
				} 
				printf ("line num : %d\n", line_number);
				line_number++;
		
			}
		}
		else {
				eax = ptrace (PTRACE_PEEKUSER, child, 4 * EAX, NULL);
				printf ("\n\n\n\n");
				insyscall_2 = 0;
			}
		}
		
		if (syscall == SYS_write) {
			long reg_val_temp[3];
			if (insyscall_3 == 0) {
				char *rule;
				insyscall_3 = 1;
				counter++;
				ptrace (PTRACE_GETREGS, child, 0, &u_in);
				reg_val_temp[1] = ptrace (PTRACE_PEEKUSER, child, 4 * EBX, NULL);
				reg_val_temp[2] = ptrace (PTRACE_PEEKUSER, child, 4 * ECX, NULL);
				reg_val_temp[3] = ptrace (PTRACE_PEEKUSER, child, 4 * EDX, NULL);
				printf ("System call is: %d\t\t/write\n", syscall);
				// Not able to print the buffer location
				// fprintf (stderr,"Pointer to buf is %ld", reg_val_temp[2]);
				fprintf(stderr, "File Descriptor is: %ld\nCount of bytes to send: %ld\n", reg_val_temp[1], reg_val_temp[3]);
				struct stat sb;
				fstat (reg_val_temp[1], &sb);
					int iterator;
				int temp = -1;
				for (iterator = 0; iterator < inode_index; iterator++) {
					if (inode_nums[iterator] == sb.st_ino ) {
						temp = 1;
						break;
					}
				}
				if (temp != -1) {
					fprintf (fp, "FD%d equals FD%d\n", counter, temp);
					sprintf (rule, "FD%d equals FD%d\n", counter, temp);
					if (model_check (line_number, rule) == -1 ) {
						exit(0);
					} 
					line_number++;
				}
				switch (sb.st_mode & S_IFMT) {
						 case S_IFLNK:  fprintf (fp,"FD%d is_symlink\n",counter);
						 		sprintf (rule, "FD%d is_symlink\n",counter);
						 		if (model_check (line_number, rule) == -1 ) {
									exit(0);
								}	 
						 		break;
				}
			}
			else {
				eax = ptrace (PTRACE_PEEKUSER, child, 4 * EAX, NULL);
				printf ("\n\n\n\n");
				insyscall_3 = 0;
			}	
		}
	
		if (syscall == SYS_close) {
			long reg_val_temp[3];
			if (insyscall_4 == 0) {
				char *rule;
				counter++;
				insyscall_4 = 1;
				ptrace (PTRACE_GETREGS, child, 0, &u_in);
				reg_val_temp[1] = ptrace (PTRACE_PEEKUSER, child, 4 * EBX, NULL);
				printf("System call is: %d\t\t/close\n Closing file with fd :%ld", syscall, reg_val_temp[1]);
				
				int temp = check_array(reg_val_temp[1], array) ;
						if (temp != -1) {
							fprintf(stderr, "Closing file with fd: %ld\n",reg_val_temp[1]);
							fprintf (fp,"FD%d equals FD%d\n", counter, temp);
							sprintf (rule, "FD%d equals FD%d\n", counter, temp);
							if (model_check (line_number, rule) == -1 ) {
								exit(0);
							} 
							line_number++;
						}
			}
			else {
				eax = ptrace (PTRACE_PEEKUSER, child, 4 * EAX, NULL);
				if (eax == 0) 
					printf ("\n\n\n\n"); //File closed successsfully
				insyscall_4 = 0;
			}	
		}
		/*
		if (syscall == SYS_execve) {
			long reg_val_temp[3];
			
			if (insyscall_5 == 0) {
				printf ("Systen call is :\t\t/execv\n");
			}
			else {
				eax = ptrace (PTRACE_PEEKUSER, child, 4 * EAX, NULL);
				if (eax == 0)			
				printf("the return in 0");
			}
		}
		*/		
	ptrace (PTRACE_SYSCALL, child, NULL, NULL);
	}
	
file_counter--;inode_index--;			// This should be done because if 10 files are opened the file counter becomes >10 hence  
printf("The open files are:\n");
int temp, temp_2;				// open_files[11] gives me a overflow
for (temp=file_counter; temp >= 0; temp--) {
	for (temp_2=0; temp_2<strlen(open_files[temp]);temp_2++) {
		printf("%c",open_files[temp][temp_2]);
	}
printf("\t%d\n", temp);
}

printf("\n\n");
printf("The arguments are :\n");
for (temp=(argc-2); temp >= 0; temp--) {
	for (temp_2=0; temp_2<strlen(arguments[temp]);temp_2++) {
		printf("%c",arguments[temp][temp_2]);
	}
printf("\t%d\n", temp);
}
printf("\n\n");
temp = 0;
for (temp=inode_index; temp >= 0; temp--) {
	printf("%1ld \t %d\n", array[temp], temp);
}
}
return 0;
}
