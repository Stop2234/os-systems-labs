#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 32

int main(int argc, char *argv[]) {
    int fd_src = -1;
    int fd_dst = -1;
    ssize_t bytes_read;
    char buffer[BUF_SIZE];

    if (argc != 3) {
        fprintf(stderr, "Использование: %s <исходный_файл> <файл_назначения>\n", argv[0]);
        return 1;
    }

    const char *src_path = argv[1];
    const char *dst_path = argv[2];

    struct stat st;

    if (stat(src_path, &st) < 0) {
        perror("stat исходного файла");
        return 1;
    }

    fd_src = open(src_path, O_RDONLY);
    if (fd_src < 0) {
        perror("open исходного файла");
        return 1;
    }

    fd_dst = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd_dst < 0) {
        perror("open файла назначения");
        close(fd_src);
        return 1;
    }

    while ((bytes_read = read(fd_src, buffer, BUF_SIZE)) > 0) {
        ssize_t total_written = 0;
        while (total_written < bytes_read) {
            ssize_t bytes_written =
                write(fd_dst, buffer + total_written, bytes_read - total_written);
            if (bytes_written < 0) {
                perror("write в файл назначения");
                close(fd_src);
                close(fd_dst);
                return 1;
            }
            total_written += bytes_written;
        }
    }

    if (bytes_read < 0) {
        perror("read из исходного файла");
        close(fd_src);
        close(fd_dst);
        return 1;
    }

    mode_t perm = st.st_mode & 0777;
    if (fchmod(fd_dst, perm) < 0) {
        perror("fchmod файла назначения");
        close(fd_src);
        close(fd_dst);
        return 1;
    }

    if (close(fd_src) < 0) {
        perror("close исходного файла");
    }
    if (close(fd_dst) < 0) {
        perror("close файла назначения");
        return 1;
    }

    return 0;
}
