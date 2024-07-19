// implement ls
//  with flag: empty, -l; -i;
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

//./UnixLs -il -ililil -lllll . .. ~

// function support
char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
void print_i(struct dirent *dirent, const char *path, int inode);
void print_l(struct dirent *dirent, const char *path, int inode);


void translateMode(struct stat stat_file){
    //process permission
    char permission[] = "?rwxrwxrwx";

    for (int i = 8; i >= 0; i--)
    {
        if (((stat_file.st_mode & (1 << i))))
        {
        }
        else
        {
            permission[9 - i] = '-';
        }
    }

    switch (stat_file.st_mode & __S_IFMT)
    {
    case __S_IFDIR:
        permission[0] = 'd';
        break;
    case __S_IFLNK:
        permission[0] = 'l';
        break;
    case __S_IFREG:
        permission[0] = '-';
        break;
    }
    printf("%-10s", permission);
}
void print_i(struct dirent *dirent, const char *path, int inode)
{
    if (inode == 1)
    {
        printf("%-7lu %s\n", dirent->d_ino, dirent->d_name);
    }
    else
    {
        printf("%s\n", dirent->d_name);
    }
};

void print_l(struct dirent *dirent, const char *path, int inode)
{
    if (inode == 1)
    {
        printf("%7lu \t", dirent->d_ino);
    }

    struct stat stat_buf;
    char path_buff[2048];
    char link_buff[2048];
    snprintf(path_buff, sizeof(path_buff), "%s/%s", path, dirent->d_name);
    lstat(path_buff, &stat_buf);


    //get grgid, pwuid
    struct group *grp = NULL;
    struct passwd *pw = NULL;
    if (!(grp = getgrgid(stat_buf.st_gid)))
    {
        perror("getgrgid\n");
    }

    if (!(pw = getpwuid(stat_buf.st_uid)))
    {
        perror("getpwuid\n");
    }

    //process time
    struct tm *time_info = localtime(&stat_buf.st_mtime);
    
    translateMode(stat_buf);
    printf("%4lu", stat_buf.st_nlink);
    printf("%12s", pw->pw_name);
    printf("%10s", grp->gr_name);
    printf("%10lu", stat_buf.st_size);
    printf("%5s %02d %04d %02d:%02d  ", months[time_info->tm_mon], time_info->tm_mday,time_info->tm_year+1900,time_info->tm_hour,time_info->tm_min);
    printf("%s", dirent->d_name);
    if(dirent->d_type == 10){
        readlink(dirent->d_name,link_buff,sizeof(link_buff));
        printf("-> %s", link_buff);
    }

    printf("\n");
};

void processDir(const char *dir_path, void (*func)(struct dirent *dirent, const char *path, int i), int inode)
{
    DIR *dir;
    int inode_p = inode;
    struct dirent *dir_dirrent;
    if ((dir = opendir(dir_path)) == NULL)
    {
        perror("Error open directory\n");
    }

    while ((dir_dirrent = readdir(dir)) != NULL)
    {
        if (dir_dirrent->d_type == 4 || dir_dirrent->d_type == 8 || dir_dirrent->d_type == 10)
        {
            if (dir_dirrent->d_name[0] == '.')
                continue;
            func(dir_dirrent, dir_path, inode_p);
        }
    }
    closedir(dir);
}
int main(int argc, char **argv)
{
    if (argc == 1)
    {
        // perform normal ls
    }

    // handle cmd
    int cmd = 1;
    int flag_i = 0;
    int flag_j = 0;

    while (cmd < argc && argv[cmd][0] == '-')
    {
        if (flag_i == 1 && flag_j == 1)
        {
            break;
        }
        int cmd_track = 0;

        while (argv[cmd][cmd_track] != '\0')
        {
            if (argv[cmd][cmd_track] == 'i')
            {
                flag_i = 1;
            }
            if (argv[cmd][cmd_track] == 'l')
            {
                flag_j = 1;
            }
            cmd_track++;
        }
        cmd++;
    }
    
    // process the directory
    if (cmd == argc){
        if (flag_i == 0 && flag_j == 0)
        {
            // normal ls
            processDir(".", print_i, 0);
        }
        else if (flag_i == 1 && flag_j == 1)
        {
            // ls -il
            processDir(".", print_l, 1);
        }
        else if (flag_i == 1 && flag_j == 0)
        {
            // ls -i
            processDir(".", print_i, 1);
        }
        else if (flag_i == 0 && flag_j == 1)
        {
            // ls -l
            processDir(".", print_l, 0);
        }
        return 1;
    }

    // skip cmd
    while (argv[cmd][0] == '-')
    {
        cmd++;
    }
    while (cmd < argc)
    {
        printf("%s: \n",argv[cmd]);
        if (flag_i == 0 && flag_j == 0)
        {
            // normal ls
            processDir(argv[cmd], print_i, 0);
        }
        else if (flag_i == 1 && flag_j == 1)
        {
            // ls -il
            processDir(argv[cmd], print_l, 1);
        }
        else if (flag_i == 1 && flag_j == 0)
        {
            // ls -i
            processDir(argv[cmd], print_i, 1);
        }
        else if (flag_i == 0 && flag_j == 1)
        {
            // ls -l
            processDir(argv[cmd], print_l, 0);
        }
        cmd++;
    }
}