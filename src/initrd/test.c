#include <dirent.h>
#include <stdio.h>
#include <string.h>

void recursive_dir_scanner(char* path, void (*callback)(char*)) {
	DIR* d = opendir(path);
	if (d) {
		struct dirent* dir;
		while ((dir = readdir(d)) != NULL) {
			char buffer[256];
			if (strcmp(".", dir->d_name) == 0 || strcmp("..", dir->d_name) == 0)
				continue;
				
			memset(buffer, 0, 256);
			strcat(buffer, path);
			if (strcmp("/", path) != 0)
				strcat(buffer, "/");
			strcat(buffer, dir->d_name);
			if (dir->d_type == 0x08) {
				callback(buffer);
			} else if (dir->d_type == 0x04) {
				recursive_dir_scanner(buffer, callback);
			}
		}
		closedir(d);
	}
}

void cb(char* path) {
	printf("%s\n", path);
}

int main(void) {
	recursive_dir_scanner("/", cb);
	return 0;
}
