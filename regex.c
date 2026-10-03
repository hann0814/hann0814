#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MATCH_NUM 3

void printregerror(int err, regex_t *reg)
{
    char errbuf[512];
    regerror(err, reg, errbuf, sizeof(errbuf));
    printf("正则错误：%s\n", errbuf);
}

int main(void)
{
    int fd;
    struct stat st;
    char *mmap_p = NULL;
    regex_t reg;
    int ret;
    regmatch_t match[MATCH_NUM];
    
    fd = open("url.html", O_RDONLY);
    if(fd < 0)
    {
        perror("open url.html failed");
        return -1;
    }
    fstat(fd, &st);
    size_t fsize = st.st_size;

    mmap_p = (char *)mmap(NULL, fsize, PROT_READ, MAP_PRIVATE, fd, 0);
    if(mmap_p == MAP_FAILED)
    {
        perror("mmap fail");
        close(fd);
        return -1;
    }
    close(fd);

    const char *pat = "<a[^>]+href=\"([^\"]+)\"[^>]*>([^<]+)</a>";
    ret = regcomp(&reg, pat, REG_EXTENDED);
    if(ret != 0)
    {
        printregerror(ret, &reg);
        munmap(mmap_p, fsize);
        return -1;
    }
    char *cur_p = mmap_p;
    while(1)
    {
        ret = regexec(&reg, cur_p, MATCH_NUM, match, 0);
        if(ret == REG_NOMATCH)
        {
            break;
        }
        if(ret != 0)
        {
            printregerror(ret, &reg);
            break;
        }

        if(match[1].rm_so != -1 && match[2].rm_so != -1)
        {
            int url_len = match[1].rm_eo - match[1].rm_so;
            char url[512] = {0};
            strncpy(url, cur_ptr + match[1].rm_so, url_len);

            int title_len = match[2].rm_eo - match[2].rm_so;
            char title[1024] = {0};
            strncpy(title, cur_ptr + match[2].rm_so, title_len);

            printf("【新闻链接】%s\n【新闻标题】%s\n------------------------\n", url, title);
        }
        cur_p += match[0].rm_eo;
    }
    regfree(&reg);
    munmap(mmap_p, fsize);
    return 0;
}
