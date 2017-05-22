#include "func.h"

#define in
#define out
#define BLK 64

struct data{
	char *cpfrom_st;
	char *cpfrom_nd;
	char *cpto_st;
	char *cpto_nd;
};

void *cpthrd(void in *p)
{
	struct data *p1 = (struct data *)p;
	char *pfs = p1->cpfrom_st;
	char *pfn = p1->cpfrom_nd;
	while((pfn>pfs) &&(pfn - pfs )>BLK ){
		memcpy(p1->cpto_st,pfs,BLK);
		p1->cpto_st +=BLK;
		pfs += BLK;
	}
	if(pfn>pfs){
		memcpy(p1->cpto_st,pfs,pfn-pfs);
	}
	printf("Job done\n");
}
int findfile(char in *name,char in out **path)
{
	int ret = 0;
	DIR *dir = opendir(*path);
	if(dir == NULL) exit(0);
	struct dirent *dt;
	while((dt = readdir(dir)) !=NULL && (ret == 0) ){
		if(!strcmp(".",dt->d_name) || !strcmp("..",dt->d_name) ) continue ;
		char *buf;
		buf = (char *)calloc(1,128);
		if(dt->d_type == 4)
		{
			sprintf(buf,"%s%s%s",*path,"/",dt->d_name);
			ret = findfile(name,&buf);
			if(ret > 0){
				*path = buf;
				return ret;
			}
		}
		if(!strcmp(dt->d_name,name))
		{
			sprintf(buf,"%s%s%s",*path,"/",dt->d_name);
			*path = buf;
			return 1;
		}	
	}
	return ret;
}
void create_file(char in out *saveas,off_t size){
	int fd = open(saveas,O_CREAT|O_RDWR,0666);
	if(-1 == fd) exit(1);
	chmod(saveas,0666);
	ftruncate(fd,size);
	close(fd);
	return ;
}

int main(){
	int i = 0,ret = 0;
	char tgtflname[20] = {0},*tgtflpth = (char *)calloc(1,128),savingas[128]={0};
	strcpy(tgtflpth,"/home/lagrange");
	printf("please input the file looking for:\n");
	scanf("%s",tgtflname);
	while(ret = findfile(tgtflname,&tgtflpth) == 0){
		free(tgtflpth);
		memset(tgtflname,20,0);
		memset(tgtflpth,128,0);
		strcpy(tgtflpth,"/home/lagrange");
		printf("please input the file looking for again:\n");
		scanf("%s",tgtflname);
	}
	printf("%s\n",tgtflpth);
	struct stat dbuf1,dbuf2;
	memset(&dbuf1,0,sizeof(dbuf1));
	memset(&dbuf2,0,sizeof(dbuf2));
	if((ret = stat(tgtflpth,&dbuf1)) < 0)	return ret;
	printf("please input the saving path:\n");
	scanf("%s",savingas);
	create_file(savingas,dbuf1.st_size);
	stat(savingas,&dbuf2);
	if((ret = stat(savingas,&dbuf2)) < 0)    return ret;
	int src = open(tgtflpth,O_RDONLY);
	int dst = open(savingas,O_RDWR);
	char *mmap_s = (char *)mmap(NULL,dbuf1.st_size,PROT_READ,MAP_SHARED,src,0);
	if(mmap_s == (char *)-1) return -2;
	char *mmap_r = (char *)mmap(NULL,dbuf2.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,dst,0);
	if(mmap_r == (char *)-1) return -2;
	struct data* pthcp = (struct data* )calloc(10 , sizeof( struct data));
	memset(pthcp,0,10*sizeof(struct data));
	for(i = 0;i<10;i++){
		pthcp[i].cpfrom_st = mmap_s + i * dbuf1.st_size / 10;
		pthcp[i].cpfrom_nd = pthcp[i].cpfrom_st + dbuf1.st_size / 10;
		pthcp[i].cpto_st = mmap_r + i * dbuf1.st_size / 10;
		pthcp[i].cpto_nd = pthcp[i].cpfrom_st + dbuf1.st_size / 10;
	}
	pthcp[9].cpfrom_nd = mmap_s + dbuf1.st_size;
	pthcp[9].cpto_nd = mmap_r + dbuf2.st_size;
	pthread_t pthid[10];
	for(int i = 0; i < 10;i ++){
		pthread_create(&pthid[i],NULL,cpthrd,&pthcp[i]);
	}
	for(int i = 0; i < 10; i++){
		ret = pthread_join(pthid[i],NULL);
		if(ret < 0)	return -1*i;
	}
	munmap(mmap_s,dbuf1.st_size);
	munmap(mmap_r,dbuf2.st_size);
	close(dst);
	close(src);
	free(pthcp);
	free(tgtflpth);
	return 0;
}
