#include <sys/wait.h>
//waitpid() and associated macros
#include <unistd.h>
//chdir()
//fork()
//exec()
//pid_t
#include <stdlib.h>
//malloc()
//realloc()
//free()
//exit()
//execvp()
//EXIT_SUCCESS, EXIT_FAILURE
#include <stdio.h>
//fprintf()
//printf()
//stderr
//getchar()
//perror()
#include <string.h>
//strcmp()
//strtok()

#include <fcntl.h>
//open()
//close()

void lsh_loop(void);
char *lsh_read_line(void);
char **lsh_split_line(char *);
int lsh_execute(char **);

int main(int argc, char **argv)
{
// Load config files, if any.
// Run command loop.
    lsh_loop();
// Perform any shutdown/cleanup.
    return EXIT_SUCCESS;
}

void lsh_loop(void)
{
    char *line;
    char **args;
    int status;
    do {
        printf("Su-Shell $ ");
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);
        free(line);
        free(args);
    } 
    while (status);
}

#define LSH_RL_BUFSIZE 1024


char *lsh_read_line()
{
    
    char *line = NULL;
    ssize_t bufsize = 0; // have getline allocate a buffer for us
    if (getline(&line, &bufsize, stdin) == -1)
    {
        if (feof(stdin)) 
        {
            exit(EXIT_SUCCESS); // We recieved an EOF
        } 
        else 
        {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
int no_elem_tok;
char **lsh_split_line(char *line)
{
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;
    if (!tokens) 
    {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, LSH_TOK_DELIM);
    
    while (token != NULL) 
    {
        tokens[position] = token;
        position++;
        if (position >= bufsize) 
        {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) 
            {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, LSH_TOK_DELIM);
    }

    tokens[position] = NULL;
    no_elem_tok = position;
    return tokens;
}

int lsh_launch(char **args)
{
    pid_t pid, wpid;
    int status;    
    pid = fork();
    if (pid == 0) 
    {
        // Child process
        if (execvp(args[0], args) == -1) 
        {
            perror("Mensaje de error:");
        }
    exit(EXIT_FAILURE);
    }

    else if (pid < 0) 
    {
    // Error forking
    perror("lsh");
    } 

    else 
    {
    // Parent process
        do 
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        }
        while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}


/*
Function Declarations for builtin shell commands:
*/
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
/*
List of builtin commands, followed by their corresponding functions.
*/
char *builtin_str[] = {"cd","help","exit"};
int (*builtin_func[]) (char **) = {&lsh_cd, &lsh_help, &lsh_exit};


int red_out_init(char * nameFile,char * type);
int red_out_end(int nameFile,int fd);
int red_in_init(char * nameFile);
int red_in_end(int nameFile,int fd);

int lsh_num_builtins() 
{
    return sizeof(builtin_str) / sizeof(char *);
}
int lsh_cd(char **args)
{
    if (args[1] == NULL) 
    {
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    } 
    else 
    {
        if (chdir(args[1]) != 0) 
        {
            perror("lsh");
        }
    }  
    return 1;
}
int lsh_help(char **args)
{
    int i;
    printf("Leonardo Ramirez Calatayud\n Naommi Lahera Champagne\nYisell Martinez Noa");
    printf("Los built in son:\n");
    
    for (i = 0; i < lsh_num_builtins(); i++) 
    {
        printf(" %s\n", builtin_str[i]);
    }
    return 1;
}
int lsh_exit(char **args)
{
    return 0;
}

char *red_mod_srt[] = {">",">>"};
int len_red_mod() 
{
    return sizeof(red_mod_srt) / sizeof(char *);
}
int lsh_execute(char **args)
{
    if (args[0] == NULL) 
    {
        // An empty command was entered.
        return 1;
    }
    int cambiofd = -1;
    int cambiofdin = -1;
    int archivo;
    int archivoin;
    int mask_result = -1;
    int result;
    int fd = dup(1);

   
    if(no_elem_tok>2)
    {
        for (int k = 0; k < len_red_mod(); k++) 
        {
            if (strcmp(args[no_elem_tok-2], red_mod_srt[k]) == 0) 
            {
                char ** newargs = (char**) malloc((no_elem_tok - 1) * sizeof(char*));
                cambiofd = 1;   
                archivo = red_out_init(args[no_elem_tok-1],red_mod_srt[k]);

               

                for (int i = 0; i < no_elem_tok - 2; i++) 
                {
                    newargs[i] = (char*) malloc((strlen(args[i]) + 1) * sizeof(char));
                    strncpy(newargs[i], args[i], strlen(args[i]));
                    newargs[i][strlen(args[i])] = '\0';
                }
                newargs[no_elem_tok - 2] = NULL;    
                args = newargs;
                no_elem_tok = no_elem_tok -2;
            }
            if (cambiofd == 1)
            {
                break;
            }
        }
    }
    if(no_elem_tok>2)
    {
        if (strcmp(args[no_elem_tok-2],"<") == 0) 
        {
            char ** newargs = (char**) malloc((no_elem_tok - 1) * sizeof(char*));
            cambiofdin = 1;   
            archivoin = red_in_init(args[no_elem_tok-1]); 
            if (archivoin == -3)
            {
                perror("Mensaje de error:");
                if (cambiofd == 1)
                {
                    red_out_end(archivo,fd);
                }
                return 1;
            }

            for (int i = 0; i < no_elem_tok - 2; i++) 
            {
                newargs[i] = (char*) malloc((strlen(args[i]) + 1) * sizeof(char));
                strncpy(newargs[i], args[i], strlen(args[i]));
                newargs[i][strlen(args[i])] = '\0';
            }
            newargs[no_elem_tok - 2] = NULL;    
            args = newargs;
            no_elem_tok = no_elem_tok -2;
        }
    }


    for (int i = 0; i < lsh_num_builtins(); i++) 
    {
        if (strcmp(args[0], builtin_str[i]) == 0) 
        {
            mask_result = 0;
            result = (*builtin_func[i])(args);
        }
    }

        if(mask_result == -1)
        {
            result = lsh_launch(args);
        }

    if(cambiofd == 1)
    {
        red_out_end(archivo,fd);
    }
    if(cambiofdin == 1)
    {
        red_in_end(archivoin,fd);
    }
    return result;
}

int red_out_init(char * nameFile,char * type)
{
    char *nombreArchivo = nameFile;
    int archivo=NULL; 
    if( type == ">")
    {
        archivo = open(nombreArchivo, O_WRONLY | O_CREAT | O_TRUNC,0644);

    } 
    else if(type == ">>")
    { 
        archivo = open(nombreArchivo, O_APPEND | O_CREAT | O_WRONLY,0644);
    }
   
    // Si por alguna razón el archivo no fue abierto, salimos inmediatamente
    if (archivo == -1) 
    {
        printf("Error abriendo archivo %s", nombreArchivo);
        return 1;
    }
    dup2(archivo,1);
    
    return archivo;
}
int red_in_init(char * nameFile)
{
    char *nombreArchivo = nameFile;
    int archivo = open(nombreArchivo, O_RDONLY ,0644);

    if (archivo == -1) 
    {
        printf("Error abriendo archivo %s", nombreArchivo);
        return -3;
    }
    dup2(archivo,0);
    return archivo;
}
int red_in_end(int nameFile,int fd)
{
    dup2(fd,0);
    close(nameFile);
    return 0;
}
int red_out_end(int nameFile,int fd)
{
    dup2(fd,1);
    close(nameFile);
    return 0;
}