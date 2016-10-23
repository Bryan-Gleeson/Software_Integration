#include <unistd.h>
#include <limits.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_GROUPS_MAX 10

int main(int argc, char **argv)
{
        system("clear");
        cusshell_intro();
        cusshell_loop();

        return EXIT_SUCCESS;
}

int c_help(char **args);
int c_exit(char **args);
int c_pwd(char **args);
int c_userinfo(char **args);
int c_ifconfig(char **args);
int c_date(char **args);
int c_cd(char **args);
void cusshell_intro();

char *builtin_str[] = {
        "help",
        "exit",
        "pw",
        "ud",
        "ifc",
        "dt",
        "cd"
};

int (*builtin_func[]) (char **) = {
        &c_help,
        &c_exit,
        &c_pwd,
        &c_userinfo,
        &c_ifconfig,
        &c_date,
        &c_cd

};

int cmd_num_builtins()
{
        return sizeof(builtin_str) / sizeof(char *);
}

void cusshell_intro()
{
        printf("welcome to the custom shell\n");
        printf("type help for more options\n");
        printf("custom shell\n");
}

int c_help(char **args)
{
        int i;
        printf("help section\n");
        printf("use these commands\n");

        for (i=0; i<cmd_num_builtins(); i++)
        {
                printf( "%s\n", builtin_str[i]);
        }

        printf("use the man command for info on other programs\n");
        printf("help section \n");
        return 1;
}
//launch a program & wait for termination
//returns 1 to continue execution
int cusshell_launch(char **args)
{
        pid_t pid,wpid;
        int status;

        pid = fork();
        if (pid == 0)
        {
                //child process
                if (execvp(args[0], args) == -1)
                {
                        perror("cusshell");
                }
                exit(EXIT_FAILURE);
        }
        else if (pid< 0)
        {
                //fork error
                perror("cusshell");
        }
        else
        {
                //parent process
                do
                {
                        wpid = waitpid(pid, &status, WUNTRACED);
                }
                while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
        return 1;
}
int cusshell_execute(char **args)
{
        int i;

        if(args[0] == NULL)
        {
                return 1;
        }

        for (i = 0; i < cmd_num_builtins(); i++)
        {
                if(strcmp(args[0], builtin_str[i]) == 0)
                {
                        return (*builtin_func[i])(args);
                }
        }

        return cusshell_launch(args);
}
//read a line from stdin
//return the line from stdin
#define CUSSHELL_RL_BUFSIZE 1024
char *cusshell_read_line(void)
{
        int bufsize = CUSSHELL_RL_BUFSIZE;
        int position = 0;
        char *buffer = malloc(sizeof(char) * bufsize);
        int c;

        //check for memory allocation
        if (!buffer)
        {
                fprintf(stderr, "customshell:allocation error\n");
                exit(EXIT_FAILURE);
        }

        while (1)
		{
                //read character
                c = getchar();

                //if EOF replace with a NULL and return
                if (c == EOF || c == '\n')
                {
                        buffer[position] = '\0';
                        return buffer;
                }
                else
                {
                        buffer[position] = c;
                }
                position ++;

                //if buffer exceeded reallocate
                if (position >= bufsize)
                {
                        bufsize += CUSSHELL_RL_BUFSIZE;
                        buffer = realloc(buffer, bufsize);
                        if(!buffer)
                        {
                                fprintf(stderr, "customshell:allocation error\n");
                                exit(EXIT_FAILURE);
                        }
                }
        }
}
//split a line into tokens
//null-terminated array of tokens

#define CUSSHELL_TOK_BUFSIZE 64
#define CUSSHELL_TOK_DELIM " \t\r\n\a"

char **cusshell_split_line(char *line)
{
        int bufsize = CUSSHELL_TOK_BUFSIZE, position = 0;
        char **tokens = malloc(bufsize * sizeof(char*));
        char *token;

        //error check for token allocation
        if (!tokens)
        {
                fprintf(stderr, "customshell allocation error\n");
                exit(EXIT_FAILURE);
        }

        token = strtok(line, CUSSHELL_TOK_DELIM);
        while (token != NULL)
        {
                tokens[position] = token;
                position++;

                if (position >= bufsize)
                {
                        bufsize += CUSSHELL_TOK_BUFSIZE;
                        tokens = realloc(tokens, bufsize * sizeof(char*));

                        if (!tokens)
                        {
                                fprintf(stderr, "customshell allocation error\n");
                                exit(EXIT_FAILURE);
                        }
                }

  token = strtok(NULL, CUSSHELL_TOK_DELIM);
        }
        tokens[position] = NULL;
        return tokens;
}

//loop to get input and execute

void cusshell_loop(void)
{
        char *line;
        char **args;
        int status;

        do
        {
                printf("Super MLG Shrekt Shell\n");
                line = cusshell_read_line();
                args = cusshell_split_line(line);
                status = cusshell_execute(args);

                free(line);
                free(args);
        }
        while (status);
}


int c_exit(char **args)
{
        return 0;

}


int c_pwd(char **args)
{
        if(args[1] == NULL)
        {
                //pwd command
                system("pwd");
        }
        else
        {
                fprintf(stderr, "customshell unexpected argument to \"pw\"\n");
        }
        return 1;
}

int c_userinfo(char **args)
{
        if (args[1] == NULL)
        {
                int num_groups, i;
                struct group *gr;
                gid_t groups[NUM_GROUPS_MAX];
                num_groups = NUM_GROUPS_MAX;

                if (getgrouplist( getlogin(), getegid(), groups, &num_groups) == -1)
                {
                        printf("group is too small %d\n", num_groups);
                        //check size of groups

                        printf("%d, %s," , geteuid(), getenv("LOGNAME"));
                }       //^display group/user ID
                for (i = 0; i < num_groups; i++)
                {
                        gr = getgrgid(groups[i]);
                        printf("%s, ", gr->gr_name);
                }
                        //display Inode
                system("ls -id");
        }
        else
        {
                fprintf(stderr, "customshell unexpected argument to \"uid\"\n");
        }
        return 1;
}
int c_ifconfig(char **args)
{
        if (args[1] == NULL)
                system("ifconfig eth0");
        else
        {
                if (args[1] !=NULL)//check for command tail
                {
                        char ifc_ethx[100]; // New var to hold specific ifc ethX selection
                        sprintf(ifc_ethx, "ifconfig %s",args[1]);
                        system(ifc_ethx);
                }
        }
        return 1;
}

//display current date
//returns 1 to continue execution

int c_date(char **args)
{
        if (args[1] == NULL)
        {
                time_t timer;
                char buffer[26];
                struct tm* tm_info;
                time(&timer);
                tm_info = localtime(&timer);

                strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
                puts(buffer);
        }

        else
        {
                fprintf(stderr, "cusshell: unexpected argument to \"pw\"\n");
        }
        return 1;
}
//change directory
int c_cd(char **args)
{
        if (args[1] == NULL)
        {
                fprintf(stderr, "cusshell: expected argument to \"cd\"\n");
        }
        else
        {
                if (chdir(args[1]) != 0)
                {
                        fprintf(stderr, "cusshell: unexpected directory parameter used.\n");
                }
        }
        return 1;
}


