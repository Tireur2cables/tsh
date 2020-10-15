#include "cd.h"

#define pwd getcwd(NULL,0)

int main(int argc,char **argv) {
	
	DIR*courant=opendir(pwd); 
	assert(courant && pwd);
	
	if(argc==0 || argc==1) printf("Aucun répertoire indiqué");
		else {
			char *dirTAR;
			if(argc==2) {
				dirTAR=argv[1];
				while(1) {
					struct dirent *d=readdir(courant);  
					if(d==NULL) break;
					if(strcmp(d->d_name,dirTAR)==0) {  // si on trouve le bon rep!
						
					}
				}
				
				
			}
			
		}
	
			  
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	return 0;
	
}
