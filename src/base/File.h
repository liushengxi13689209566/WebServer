/*************************************************************************
	> File Name: File.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年11月20日 星期二 11时54分35秒
 ************************************************************************/

#ifndef _FILE_HPP
#define _FILE_HPP
#include "./base.hpp"

#include <sys/stat.h>

class File
{
  public:
    File(){};
    File(const File &file) = delete;
    File &operator=(const File &) = delete;

    explicit File(const char *path, mode_t mode = O_RDONLY)
    {
        printf("打开文件\n");
        fd_ = open(path, mode);
        if (fd_ > 0)
        {
            stat(path, &file_stat);
        }
    }
    ~File()
    {
        printf("关闭文件描述符\n");
        close(fd_);
    }
    inline int GetFileFd()
    {
        return fd_; /*返回－1,代表文件不存在*/
    }
    inline off_t Size()
    {
        return file_stat.st_size;
    }
    inline bool IsForbid()
    {
        return !(file_stat.st_mode & S_IROTH);
    }
    inline bool IsDir()
    {
        return S_ISDIR(file_stat.st_mode);
    }

  private:
    int fd_ = 0;
    struct stat file_stat = {0};
};
#endif
