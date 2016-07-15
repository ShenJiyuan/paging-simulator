#include "cse356header.h"
#define  MAX 100000
 
//*****************************************************结构体定义和全局变量声明************************************************************
struct page
{
    char page[20];
    char name[20];
    int used;       //LRU算法
    int second;     //second-chance算法
    double count;      //是否被使用过:>0_可被置换；0_不可被置换.
    double occur;      //my-alg按照页使用次数
};
struct process   
{
    char name[20];//process's name
    double arrivetime;
    double CPUtime;
    double needtime; 
    double rearrivetime;
    double elapsedtime;
    int IOops;
    int pageFault;
    char state[20];
    FILE*src;
    struct process *next;
};
//global  variables
int first=0;        //FIFO的first位置
int lru=0;          //LRU计数
int sec=0;          //Second-Chance位置
//int last;           //my-alg
double effecttime=0;     //运行时间
double starttime=0;
double totaltime=0;
bool startflag=false;
double clocktime=0;//mark the important time
struct process *readyhead=(struct process*)malloc(sizeof(struct  process));
struct process *readytail=readyhead;
struct process *blockhead=(struct process*)malloc(sizeof(struct  process));
struct process *blocktail=blockhead;
struct process *finishhead=(struct process*)malloc(sizeof(struct  process));
struct process *finishtail=finishhead;

//**********************************************************create函数***********************************************************************
void create(char *openFile)
{
    char tmp[512]; //定义一个临时变量，存储每一行数据
     
    FILE *infile;
    infile=fopen(openFile,"r");
    if(infile==NULL)
    {
        printf("\nFailed to open the file");
        exit(1);
    }
    while( fgets( tmp, sizeof(tmp), infile ) ) //读每一行，创建一个proces*入readyqueue
    {
        struct process *p=(struct process*)malloc(sizeof(struct  process));
        sscanf( tmp,"%s %lf %lf %d",p->name,&(p->arrivetime),&(p->CPUtime),&(p->IOops) ); 
        p->arrivetime=p->arrivetime*100000;
        p->CPUtime=p->CPUtime*100000;
        p->needtime=p->CPUtime;
        p->pageFault=0;
        strcpy(p->state,"ready");
        p->rearrivetime=p->arrivetime;
        p->next=NULL;
        readytail->next=p;
        readytail=p;
        readytail->next=NULL;
       //~~~~~~~~~~~~~~~~~~~~~打开进程mem文件~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        char open[20];
        strcpy(open,p->name);
        strcat(open,".mem");
        p->src=fopen(open,"r");
        if(p->src==NULL)
        {
            printf("\nFailed to open the file TTT");
            exit(1);
        }
    }
    struct process *ppp=readyhead->next;  
 /*   printf("=============readyQueue=============\n");
    printf("name\tarrivetime\tCPUtime\t\tstate\tIOops\tpagefault\n");
    while(ppp!=NULL)
    {
       printf("%s\t%lf\t%lf\t%s\t%d\t%d\n",ppp->name,ppp->arrivetime,ppp->CPUtime,ppp->state,ppp->IOops,ppp->pageFault);
       ppp=ppp->next;
    }*/
       
    fclose(infile);
}

//***********************************************************insert函数***********************************************************************
void insert(struct process *p)
{
    struct process *s;
    s=readyhead;
    while((s->next!=NULL)&&(s->next->rearrivetime<=p->rearrivetime))
    {
        s=s->next;
    }
    p->next=s->next;
    s->next=p;
    if (p->next==NULL) readytail=p;
    strcpy(p->state,"ready");
    printf("%s into ready queue.\n",p->name);
}

//*************************************************************block函数*********************************************************************
void block(struct process *p)
{
   p->next=NULL;
   blocktail->next=p;
   blocktail=p;
   strcpy(p->state,"blocking");
   printf("%s out of ready queue.\n",p->name);
   printf("%s into block queue.\n",p->name);
}

//************************************************************deblock函数********************************************************************
void deblock()
{
   if (blockhead->next!=NULL)
   {
      blockhead->next->rearrivetime=clocktime;
      struct process*yuan=blockhead->next;
      blockhead->next=yuan->next; 
      yuan->next=NULL;
      printf("%s out of block queue.\n",yuan->name);     
      insert(yuan);
   }
}
 
//***********************************************************compare函数*********************************************************************
bool compare(struct page* memoryi,char* temp,char*name)
{
     if ((strcmp(memoryi->name,name)==0) && (strcmp(memoryi->page,temp)==0)) return true;
     else return false;
}

//**********************************************************pagefault函数********************************************************************
int pagefault(struct page* memory,char *temp,char *name,int memsize,int len, char* pr)
{
   bool replaceFlag=false;
   if (len<memsize) 
   { 
     strcpy(memory[len].page,temp);
     strcpy(memory[len].name,name);
     memory[len].used=lru;lru=lru+1;
     memory[len].second=1;
     memory[len].count=0;      
     len=len+1;
     replaceFlag=true;
   }
   else 
   {
      char ch=pr[0];
      switch(ch)
      {
         case 'f':{int d=0;bool shen=true;
                  while (memory[first].count==0) 
                         {first=(first+1)%memsize;d++;
                          if (d==memsize) {shen=false;break;}}
                  if (shen==true)
                  { strcpy(memory[first].page,temp);
                    strcpy(memory[first].name,name);
                    memory[first].count=0;
                    first=(first+1)%memsize;
                    replaceFlag=true;}
                  break;}
         case 'l':{int k;
                  int least;
                  for (least=0;least<memsize;least++) if (memory[least].count!=0) break;
                  if (least<memsize)
                  { for (k=least;k<memsize;k++) 
                     if ((memory[k].used<memory[least].used) && (memory[k].count!=0))
                         least=k;
                    strcpy(memory[least].page,temp);
                    strcpy(memory[least].name,name);
                    memory[least].count=0;
                    memory[least].used=lru;
                    lru++;
                    replaceFlag=true;
                  }
                  break;}
         case '2':{ 
                   int g=0;int s=0;bool ch=true;
                   while ((memory[sec].second==1) || (memory[sec].count==0))
                     {
                        g++;
                        if (memory[sec].second==1)
                          memory[sec].second=0;
                        if (memory[sec].count==0)
                            s++;
                        if ((g==memsize) && (s==g)) {ch=false;break;}
                        sec=(sec+1)%memsize;
                     }
                  if (ch==true)
                  {
                    strcpy(memory[sec].page,temp);
                    strcpy(memory[sec].name,name);
                    memory[sec].count=0;
                    memory[sec].second=1;
                    sec=(sec+1)%memsize;
                    replaceFlag=true;
                  }
                  break;}
         case 'n':{/*strcpy(memory[last].page,temp);
                  strcpy(memory[last].name,name);
                  last=(last+1+memsize/2)%memsize;
                  replaceFlag=true;*/
//--------------------------------------------------------
                  int q;
                  int last;
                  last=0;
                  for (last=1;last<memsize;last++) if (memory[last].count!=0) break;
                  if (last<memsize)
                  { for (q=last;q<memsize;q++) 
                     if ((memory[q].occur<memory[last].occur) && (memory[q].count!=0))
                         last=q;
                    strcpy(memory[last].page,temp);
                    strcpy(memory[last].name,name);
                    replaceFlag=true;
                    memory[last].occur=0;
                    memory[last].count=0;
                  }
                  break;}
         case 'm':{
                   
                  int cc;
                  int object;
                  //least=0;
                  for (object=0;object<memsize;object++) if (memory[object].count!=0) break;
                  if (object<memsize)
                  { 
                    for (object=0;object<memsize;object++) 
                         if ((memory[object].count!=0) && (strcmp(memory[object].name,name)!=0)) break;
                    //-----------------------------------------------------------------------------
                    if (object==memsize) 
                        {for (object=0;object<memsize;object++) if (memory[object].count!=0) break;
                         for (cc=object;cc<memsize;cc++) 
                             if ((memory[cc].used>memory[object].used) && (memory[cc].count!=0))
                              object=cc;
                        }
                    else
                        {
                           for (cc=object;cc<memsize;cc++) 
                             if ((memory[cc].used<memory[object].used) && (memory[cc].count!=0) && (strcmp(memory[cc].name,name)!=0))
                               object=cc;
                        }
                    //-----------------------------------------------------------------------------
                    strcpy(memory[object].page,temp);
                    strcpy(memory[object].name,name);
                    memory[object].count=0;
                    memory[object].used=lru;
                    lru++;
                    replaceFlag=true;
                  }

                  break;}
         default: break;
      }
   }

   struct process*running=readyhead->next;
   readyhead->next=running->next;
   //running->next=NULL;
   fseek(running->src,-4,SEEK_CUR);   
   if (replaceFlag==false)
    {  
        //struct process*u=blockhead;
        //while(u->next!=NULL) {printf("%s",u->next->name);u=u->next;}
        //printf("\n");
        printf("%s page replace failed.\n",running->name);
        clocktime+=50;
        block(running);
    }
   else 
     {
      printf("%s page replace succeeded.\n",running->name);
      running->rearrivetime=clocktime+1000;
      if ((readyhead->next!=NULL) && ((running->rearrivetime)>=(readyhead->next->rearrivetime)))
           {
            running->rearrivetime+=50;
            clocktime+=50;
           } 
     insert(running);
     }
   return len;
}

//****************************************************************RoundRobin函数*************************************************************
void RoundRobin(double quantum,int memsize,struct page* memory,char *pr)
{
    
    printf("=====================Scheduling======================\n");
    int len=0;
    
    while ((readyhead->next!=NULL) || (blockhead->next!=NULL))
    {
       if (readyhead->next==NULL)
          deblock();
       if (readyhead->next!=NULL)
       {
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~进程needtime与quantum比较1~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        if(readyhead->next->needtime<=quantum)
        {    
        //   printf("======needtime <= quantum==========================\n");
        //=============================开始时间=================================
        if(clocktime<readyhead->next->rearrivetime)
        {   
             clocktime=readyhead->next->rearrivetime;   
        }
        if (startflag==false) {starttime=clocktime;startflag=true;}
        //=============================运行过程=================================
             int i,j;char temp[5];bool faultflag=false;bool flag;
             int need;need=readyhead->next->needtime;
             //-----------------------------------------------------------------
             for (j=0;j<need;j++)
             {
                 flag=false;
                 fscanf(readyhead->next->src,"%s",temp);
                 if (len ==0 ) {faultflag=true;break;}
                 else 
                 { for (i=0;i<len;i++)
                     {
                       if (compare(&(memory[i]),temp,readyhead->next->name)==true)
                       {    memory[i].used=lru;lru++;
                            memory[i].second=1;
                            memory[i].occur+=1;
                            memory[i].count+=1; //printf("%lf\t%s",memory[i].count,readyhead->next->name);
                            flag=true;
                           // break;
                        }
                     }
                   if (flag==false) {faultflag=true;break;}
                   else 
                   {   clocktime+=1;
                       readyhead->next->needtime-=1;  
                       effecttime+=1;}
                 }
              } 
            //-----------------------------------------------------------------
            if (faultflag==true) 
            {
                printf("%s page fault.\n",readyhead->next->name);
                readyhead->next->pageFault+=1;
                struct process*t=readyhead->next; 
                len=pagefault(memory,temp,t->name,memsize,len,pr);
            }             
            else
             {
             readyhead->next->elapsedtime=clocktime-(readyhead->next->arrivetime);  
             finishtail->next=readyhead->next;
             finishtail=finishtail->next;                
             readyhead->next=readyhead->next->next; 
             finishtail->next=NULL;                         
             strcpy(finishtail->state,"finish");
             deblock();
             printf("%s into finish queue.\n",finishtail->name);
             if (readyhead->next!=NULL) clocktime+=50;                
             }      
          }            
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~进程needtime与quantum比较2~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        else
        {      
        //    printf("======needtime > quantum==========================\n");
        //=========================开始时间===============================
        if(clocktime<readyhead->next->rearrivetime)
        {   
             clocktime=readyhead->next->rearrivetime;   
        }
        if (startflag==false) {starttime=clocktime;startflag=true;}
        //=========================运行过程===============================
             int i,j;char temp[5];bool faultflag=false;bool flag;
             //-----------------------------------------------------------
             for (j=0;j<quantum;j++)
             {
                 fscanf(readyhead->next->src,"%s",temp);
                 flag=false;
                 if (len ==0 ) {faultflag=true;break;}
                 else 
                   { for (i=0;i<len;i++)
                     if (compare(&(memory[i]),temp,readyhead->next->name)==true)
                         { 
                              memory[i].used=lru;lru++;
                              memory[i].second=1;
                              memory[i].occur+=1;
                              memory[i].count++;    //printf("%lf\t%s",memory[i].count,readyhead->next->name);
                              flag=true;
                              //break;
                          }
                     if (flag==false) {faultflag=true;break;}  
                     else 
                       {
                       clocktime+=1;
                       readyhead->next->needtime-=1;  
                       effecttime+=1;                          
                       }
                   } 
             }
             //------------------------------------------------------------
             if (faultflag==false)
             {
             struct process*t=readyhead->next;      
             readyhead->next=t->next;  
             deblock();
             //-------------判断是否要context switch---------------------
             t->rearrivetime=clocktime; 
             if (readyhead->next!=NULL && (t->rearrivetime>=(readyhead->next->rearrivetime)))
                     {
                        t->rearrivetime+=50;
                        clocktime+=50; 
                     }   
              insert(t);
             }
             else {
                   printf("%s page fault.\n",readyhead->next->name);
                   readyhead->next->pageFault+=1;
                   struct process*t=readyhead->next; 
                   len=pagefault(memory,temp,t->name,memsize,len,pr);             
             }                
         }   
       }
    }
    totaltime=clocktime-starttime;
    printf("========in the finish queue====================\n");
    printf("name\tarrivetime\telapsedtime\t\tstate\tpagefault\n");
    struct process *pp=finishhead->next;
    while(pp!=NULL)
    {
       printf("%s\t%lf\t%lf\t%s\t%d\n",pp->name,pp->arrivetime,pp->elapsedtime,pp->state,pp->pageFault);
       pp=pp->next;
    }
}

//************************************************************main函数***********************************************************************
int main(int argc,char *argv[])
{
     //初始readyQueue
     create(argv[4]);
     //读quantum,memsize,pr-policy,建memory
     char *pr_policy=argv[3];
     if ((strcmp(pr_policy,"fifo")!=0) && (strcmp(pr_policy,"lru")!=0) && (strcmp(pr_policy,"2ch-alg")!=0) && (strcmp(pr_policy,"my-alg")!=0))
       {
          printf("Error page replacement algorithm Input!\n");
          return 0;
       }
     double quantum=atof(argv[2]);
     if (quantum<=0) {printf("Error quantum Input!\n");return 0;}
     int memsize=atof(argv[1]);
     if (memsize<=0) {printf("Error memory size Input!\n");return 0;}
     struct page *memory=(struct page *)malloc(memsize*sizeof(struct page));
     int rr;
     for (rr=0;rr<memsize;rr++)
      {
       memory[rr].count=1;
       memory[rr].occur=0;
      }
     //RoundRobin调度
     RoundRobin(quantum,memsize,memory,pr_policy);
     //打印结果
     printf("========total elapsed cycles========\n");
     printf("%lf\n",totaltime);
     printf("========total effected cycles=======\n");
     printf("%lf\n",effecttime);
     printf("==========total idle cycles=========\n");
     printf("%lf\n",totaltime-effecttime);
     int pagesfault=0;
     struct process*op=finishhead;
     while (op->next!=NULL) {pagesfault+=op->next->pageFault;op=op->next;}
     printf("==========total pagefaults==========\n");
     printf("%d\n",pagesfault);
     return 0;
}
