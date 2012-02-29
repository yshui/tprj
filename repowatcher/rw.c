#include <stdio.h>
#include <errno.h>
#include <sys/inotify.h>
#include <string.h>
#include <stdlib.h>
struct inotify_event *inev;
char *buf;
int main(int argc, char **argv){
	if(argc!=2){
		fprintf(stderr, "Usage: %s <repo path>\n", argv[0]);
		exit(1);
	}
	int infd = inotify_init();
	if(infd == -1){
		perror("Cannot initialize inotify: ");
		exit(1);
	}
	int wd = inotify_add_watch(infd, argv[1], IN_DELETE | IN_CREATE);
	int ll;
	int length = 10;
	buf = malloc(length);
	while(ll = read(infd, buf, length)){
		if(ll == -1){
			if(errno == EINVAL){
				free(buf);
				buf = malloc(length * 2);
				length *= 2;
				continue;
			}
		}
		inev = (struct inotify_event *)buf;
		if(inev->mask & IN_DELETE){
			printf("D %s\n", inev->name);
			fflush(stdout);
		}
		else if(inev->mask & IN_CLOSE_WRITE){
			inotify_rm_watch(infd, inev->wd);
			printf("C %s\n", inev->name);
			fflush(stdout);
		}
		else if(inev->mask & IN_CREATE){
			int d=strlen(argv[1]);
			char *name = malloc(sizeof(char)*(d+inev->len+1));
			strcpy(name, argv[1]);
			name[d] = '/';
			name[d+1] = '\0';
			strcat(name, inev->name);
			inotify_add_watch(infd, name, IN_CLOSE_WRITE);
		}
	}
	return 0;
}
