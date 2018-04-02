/*--------------------------------------------------------------------*/
/* prog2_temp.c - This is a partial solution, or template, for the    */
/* second programming assignment for CSCI4500, Spring 2018. It is     */
/* reasonably well-commented, and those places in which additional    */
/* code must be provided are mared with comments like this:           */
/*      XXX - TO BE WRITTEN                                           */
/* Of course there may be additional places in the code that may be   */
/* modified, deleted, or additional code added. The decision of what  */
/* to keep, toss, or modify is left to the user.                      */
/*                                                                    */
/* REMEMBER THAT THIS IS *NOT* A COMPLETE SOLUTION, NOR WILL IT YIELD */
/* REASONABLE RESULTS IF COMPILED AND EXECUTED.                       */
/*                                                                    */
/* Usage:    prog2 [-v] [inputfilename]                               */
/*--------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSTEP 50    /* max action steps for any process */
#define MAXPROC 50    /* max processes in any simulation */
#define MAXRSRC 50    /* max resources in any simulation */

FILE *f;        /* input stream */


/*----------------------------------------------------------------*/
/* There are several arrays used to record information about the  */
/* processes and the resources. Since process IDs and resoure IDs */
/* start with 1, the 0th entry in each array is unused. That is,  */
/* the arrays are treated as if they are 1-origin arrays, and not */
/* 0-origin arrays. This makes the code easier to understand at   */
/* the cost of one array entry.                                   */
/*----------------------------------------------------------------*/

/*------------------------------------------------------------------*/
/* This is an array of structures with one entry for every process. */
/* Each entry contains the "program" that will be executed by the   */
/* process (ns, a, and n), the index in the a and n arrays of the   */
/* next action to be taken, the state of the process, the total     */
/* time used by the process, and the time when it ended.            */
/*------------------------------------------------------------------*/
struct process {
    int ns;        /* # of actions for this process */
    char a[MAXSTEP];    /* actions */
    int n[MAXSTEP];    /* parameters (resource ID or time) */
    int ip;        /* index to next action */
    int state;        /* process state */
    /* -1 = finished */
    /* 0 = ready (or running) */
    /* 1..nr = blocked, waiting on resource */
    int runtime;    /* time used */
    int endtime;    /* time process ended */
} proc[MAXPROC+1];

int trace;        /* trace option value */

int simno;        /* simulation number */
int t;            /* simulation time */
int np;            /* total # of processes (1..MAXPROC) */
int nr;            /* total # of resources (1..MAXRSRC) */

/*--------------------------------------------------------------------*/
/* The following two items represent a very simple queue that records */
/* the "ready" processes (actually, the indices in the "proc" array   */
/* of the ready processes). The number of entries in the queue is     */
/* nready. If nready > 0, then the first entry in the queue is in     */
/* ready[0], and the last entry is in ready[nready-1]. This could be  */
/* made a lot "swifter" with dynamic allocation (i.e. using a linked  */
/* list for the queue), but in this case, we're not so concerned with */
/* speed.                                                             */
/*--------------------------------------------------------------------*/
int nready;        /* # of ready processes (0..np) */
int ready[MAXPROC];    /* the ready queue (each in 1..np) */


int running;        /* ID of the running process (1..np) */

/*------------------------------------------------------------------*/
/* The next three arrays are used to record information about the   */
/* resources. rstate[r] is 0 if resource r is not being used. If    */
/* resource r is being used by process p, then rstate[r] = p.       */
/* If there are any processes waiting for the resource, then their  */
/* process IDs will appear on a queue associated with the resource  */
/* being awaited. For example, suppose process 3 "owns" resource 4, */
/* and processes 1 and 2 (in that order) have attempted to gain     */
/* mutually-exclusive access to the resource. We'd then have this:  */
/*    rstate[4] = 3        process 3 owns resource 4           */
/*    nrw[4] = 2        2 procs are waiting for resource 4  */
/*    rw[4][0] = 1        process 1 is first waiting proc     */
/*    rw[4][1] = 2        process 2 is second waiting proc    */
/*------------------------------------------------------------------*/
int rstate[MAXRSRC+1];    /* resource state */
/* 0 = unused */
/* 1..np = owned by process */
int rw[MAXRSRC+1][MAXPROC+1];    /* queues of waiting processes */
int nrw[MAXRSRC+1];        /* # of procs on each queue */

/*---------------------------------------------------------------*/
/* Node of the resource graph. There is an edge from a resource  */
/* node to a process node to indicate the process "owns" the     */
/* resource. An edge from a process to a resource indicates the  */
/* process is awaiting the resource. Thus there is at most one   */
/* outgoing edge from each node.                                 */
/*---------------------------------------------------------------*/
/* Subscripts 0 to np-1 correspond to processes 0 to np-1.       */
/* Subscripts np to np+nr-1 correspond to resources 0 to nr-1.   */
/*---------------------------------------------------------------*/
struct node {        /* process/resource node */
    int e;        /* in a process node, e = index of awaited resource */
    /* in a resource node, e = index of owning process */
    /* if no edge exists, then e = -1. */
    int v;        /* has node been visited yet? */
    
} prn[MAXPROC+MAXRSRC];

/*-----------------------------------------------*/
/* Display the state of processes and resources. */
/* This function is optional, and is used for    */
/* insight into the data structures and the      */
/* diagnosis of flawed code.                     */
/*-----------------------------------------------*/
void statedump(void)
{
    int i, j;
    
    printf("State at time %d:\n", t);
    printf("Processes:\n");
    for(i=1;i<=np;i++) {
        printf("    Process %d: ", i);
        switch(proc[i].state) {
            case -1: printf("finished\n"); break;
            case 0: printf("ready/running\n"); break;
            default: printf("waiting on resource %d\n", proc[i].state); break;
        }
    }
    printf("\nResources:\n");
    for(i=1;i<=nr;i++) {
        printf("    Resource %d: ", i);
        if (rstate[i] == 0)
            printf("unused\n");
        else {
            printf("owned by process %d; ", rstate[i]);
            if (nrw[i] == 0)
                printf("awaited by no processes.\n");
            else if (nrw[i] == 1)
                printf("awaited for by process %d\n", rw[i][0]);
            else {
                printf("awaited for by processes %d", rw[i][0]);
                for(j=1;j<nrw[i];j++)
                    printf(", %d", rw[i][j]);
                putchar('\n');
            }
        }
    }
    printf("--------------------------------\n");
}

/*---------------------------------------------------------------------------*/
/* Get the next simulation and return 1; return 0 at end of file, -1 on err. */
/*---------------------------------------------------------------------------*/
/* This input function is very agressive in its attempt to detect bad input. */
/* As the input data is guaranteed to be correct, student solutions could    */
/* have substantially less code here. But, since this works, and is provided */
/* for your use, there probably isn't much reason to change it. Obviously,   */
/* you should verify you understand what it's doing!                         */
/*---------------------------------------------------------------------------*/
int getinput(void)
{
    int i, j;
    int r;                /* fscanf result */
    int c;                /* an action letter (L, U, or C) */
    int v;                /* value for the action */
    int oldv;                /* used for overflow test */
    int got1;                /* did we get at least one digit? */
    
    r = fscanf(f,"%d%d",&np,&nr);    /* # processes, # resources */
    if (r != 2) {
        fprintf(stderr,"Error reading np and nr.\n");
        return -1;
    }
    
    if (np == 0 && nr== 0) return 0;    /* end of input? */
    
    for (i=1;i<=np;i++) {        /* get data for processes 1 ... np */
        r = fscanf(f,"%d",&proc[i].ns);    /* # of steps for process i */
        if (r != 1) {
            fprintf(stderr,"Error reading number of actions for process %d\n",
                    i);
            return -1;
        }
        
        c = fgetc(f);
        for (j=0;j<proc[i].ns;j++) {        /* get action steps */
            while (c == ' ' || c == '\t')    /* skip blanks and tabs */
                c = fgetc(f);
            if (c != 'L' && c != 'U' && c != 'C') {
                fprintf(stderr,"Bad action for process %d step %d.\n", i, j);
                return -1;
            }
            proc[i].a[j] = c;            /* save action for step j */
            
            got1 = 0;                /* we've got no digits yet */
            v = 0;
            c = fgetc(f);
            while(isdigit(c)) {
                got1 = 1;
                oldv = v;
                v = v * 10 + c - '0';
                if ((v - c + '0') / 10 != oldv) {
                    fprintf(stderr,
                            "Overflow reading n for process %d step %d.\n",
                            i, j);
                    return -1;
                }
                c = fgetc(f);
            }
            if (!got1) {
                fprintf(stderr,"Missing value for process %d step %d\n", i, j);
                return -1;
            }
            proc[i].n[j] = v;
            switch(proc[i].a[j]) {
                case 'L':
                case 'U':
                    if (proc[i].n[j] < 1 || proc[i].n[j] > nr) {
                        fprintf(stderr,"Bad value for process %d "
                                "step %d.\n", i, j);
                        return -1;
                    }
                    break;
                case 'C':
                    if (proc[i].n[j] < 1) {
                        fprintf(stderr,"Bad value for process %d "
                                "step %d.\n", i, j);
                        return -1;
                    }
                    break;
                default:    /* this should not be possible */
                    fprintf(stderr,"Unrecognized action character for "
                            "process %d step %d.\n", i, j);
                    return -1;
            }
        }
        
        while (c == ' ' || c == '\t')    /* skip trailing blanks, tabs */
            c = fgetc(f);
        if (c != '\n') {
            fprintf(stderr,"Unrecognized input after actions for "
                    "process %d.\n", i);
            return -1;
        }
    }
    return 1;
}

/*-------------------------------------------------------------------*/
/* Deadlock detection algorithm.                                     */
/* See if deadlock exists in the current state.                      */
/*                                                                   */
/* If deadlock is detected, display the appropriate information (the */
/* identity of the processes and resources involved) and return 1.   */
/*                                                                   */
/* If deadlock is not detected, return 0.                            */
/*-------------------------------------------------------------------*/
/* The argument s is the subscript of the node in the resource graph */
/* with which the cycle detection algorithm begins.                  */
/*-------------------------------------------------------------------*/
/* HIGHLY RECOMMENDED: READ THE FILE NAMED 'cycle.txt' in the        */
/* csci4500 directory on Loki. Make certain you understand how the   */
/* algorithm works before attempting to implement it!                */
/*-------------------------------------------------------------------*/
int cycle(int s)
{
    
    /* XXX - TO BE WRITTEN */
    
    return 0;            /* DUMMY FUNCTION -- ALWAYS RETURNS 0 */
}

/*---------------------------------------------------------*/
/* Display IDs of processes in cycle starting with node i. */
/*---------------------------------------------------------*/
void putpcycle(int i)
{
    /* XXX - TO BE WRITTEN */
}

/*---------------------------------------------------------*/
/* Display IDs of resources in cycle starting with node i. */
/*---------------------------------------------------------*/
void putrcycle(int i)
{
    /* XXX - TO BE WRITTEN */
}

/*----------------------------------------------------------------*/
/* Build a resource graph representing the current system state.  */
/* Then check for a deadlock starting with each process node.     */
/*                                                                */
/* If a deadlock is detected, display the processes and resources */
/* involved in the deadlock and return any non-zero value.        */
/*                                                                */
/* If *NO* deadlock is detected, return 0.                        */
/*----------------------------------------------------------------*/
/* Note that a solution might choose to keep the resource graph   */
/* "up to date" as the simulation of the processes continues.     */
/* That approach would likely be faster than rebuilding the graph */
/* after each action, but we're not concerned with speed here.    */
/* And this approach is less likely to have errors...             */
/*----------------------------------------------------------------*/
int deadlock(void)
{
    int i;
    int dproc[MAXPROC];        /* IDs of processes involved in a deadlock */
    int ndproc;
    int drsrc[MAXRSRC];        /* IDs of resources involved in a deadlock */
    int ndrsrc;
    
    /*-------------------------------*/
    /* Construct the resource graph. */
    /*-------------------------------*/
    
    /* XXX - TO BE WRITTEN */
    
    /*-----------------------------------------------------------------*/
    /* Check for a cycle starting at every possible node in the graph. */
    /*-----------------------------------------------------------------*/
    for(i=0;i<np;i++)    /* check for a cycle starting at each process node */
        if (cycle(i)) {    /* Display the cycle */
            printf("Deadlock detected at time %d involving...\n", t);
            putpcycle(i);
            putrcycle(i);
            return 1;        /* report deadlock detected */
        }
    
    return 0;            /* report no deadlock detected */
}

/*-----------------------------------------------------*/
/* Add process p (1..np) to the end of the ready queue */
/*-----------------------------------------------------*/
void makeready(int p)
{
    ready[nready++] = p;
}

/*--------------------------------------------------*/
/* Add process p (1..np) to the end of the queue of */
/* processes waiting on resource r (1..nr)          */
/*--------------------------------------------------*/
void makewait(int p, int r)
{
    rw[r][nrw[r]] = p;
    nrw[r]++;
}

/*---------------------------------------------------------------*/
/* Process options. Then read input data, simulate and check for */
/* deadlock, and then repeat until end of input.                 */
/*---------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int i, ip, n;
    int dd;                /* non-zero if deadlock detected */
    char a;
    
    /*-----------------*/
    /* Handle options. */
    /*-----------------*/
    while (argc > 1 && argv[1][0] == '-') {
        if (!strcmp(argv[1],"-v")) {
            trace++;
            argc--;
            argv++;
            continue;
        }
        fprintf(stderr,"Unknown option %s\n", argv[1]);
        exit(1);
    }
    
    /*------------------------------------*/
    /* Setup stream f for the input data. */
    /*------------------------------------*/
    if (argc > 2) {
        fprintf(stderr,"Usage: prog2 [-v] [inputfilename]\n");
        exit(1);
    }
    if (argc == 2) {
        f = fopen(argv[1],"r");
        if (f == NULL) {
            fprintf(stderr,"Cannot open %s for input.\n", argv[1]);
            exit(1);
        }
    } else
        f = stdin;
    
    for (simno=1;;simno++) {
        /*---------------------------------*/
        /* Get the next set of input data. */
        /*---------------------------------*/
        if (getinput() != 1)
            break;
        t = 0;                /* set simulation time */
        
        /*---------------------------------*/
        /* Initialize the data structures. */
        /*---------------------------------*/
        for (i=1;i<=np;i++) {        /* initialize each process */
            proc[i].ip = 0;            /* first action index */
            proc[i].state = 0;            /* process state is ready */
            proc[i].runtime = 0;        /* no time used yet */
            ready[i-1] = i;            /* setup initial ready queue */
        }
        nready = np;
        
        for (i=1;i<=nr;i++) {        /* initialize each resource */
            rstate[i] = 0;            /* unused */
            nrw[i] = 0;                /* no waiting processes */
        }
        
        printf("Simulation %d\n", simno);
        
        /*-----------------------------------------------------------*/
        /* Perform deadlock detection and simulate a process action. */
        /*-----------------------------------------------------------*/
        for(;;) {
            dd = deadlock();        /* check for deadlock */
            if (dd)            /* if it was detected */
                break;
            
            /*---------------------------------------------*/
            /* Get a process from the ready queue to run.  */
            /* If there are no ready processes, we must be */
            /* done or deadlocked.                         */
            /*---------------------------------------------*/
            if (nready == 0) break;        /* no ready processes */
            running = ready[0];            /* first ready process */
            for (i=1;i<nready;i++)        /* slow queue removal, */
                ready[i-1] = ready[i];        /* but who cares? */
            nready--;
            
            /*--------------------------------------*/
            /* Get ip, a, and n for running process */
            /*--------------------------------------*/
            ip = proc[running].ip;
            a = proc[running].a[ip];
            n = proc[running].n[ip];
            
            if (trace) {
                printf("%d: ", t);
                printf("process %d: ", running);
                printf("%c%d\n", a, n);
            }
            
            /*--------------------------------------------*/
            /* If the process is requesting a resource... */
            /*--------------------------------------------*/
            if (a == 'L') {
                /*------------------------------*/
                /* If the resource is available */
                /*------------------------------*/
                if (rstate[n] == 0) {
                    
                    /* XXX - TO BE WRITTEN */
                    
                    /*-----------------------------------*/
                    /* If the resource is not available. */
                    /* Time does NOT increase here!      */
                    /*-----------------------------------*/
                } else {
                    if (trace) printf("\t(resource %d unavailable)\n", n);
                    makewait(running,n);        /* add to waiters */
                    proc[running].state = n;        /* mark proc blocked */
                }
            }
            
            /*-------------------------------------------*/
            /* If the process is releasing a resource... */
            /*-------------------------------------------*/
            else if (a == 'U') {
                rstate[n] = 0;        /* resource unused now */
                if (trace) printf("\t(resource %d released)\n", n);
                
                /*---------------------------------------------*/
                /* If any processes are waiting on the reource */
                /*---------------------------------------------*/
                if (nrw[n] > 0) {
                    /*--------------------------------------*/
                    /* Make the first waiting process ready */
                    /*--------------------------------------*/
                    if (trace) printf("\t(process %d unblocked)\n", rw[n][0]);
                    
                    /* XXX - TO BE WRITTEN */
                }
                
                /*----------------------------------------*/
                /* The currently running process advances */
                /*----------------------------------------*/
                if (ip+1 == proc[running].ns) {
                    proc[running].state = -1;        /* done */
                    proc[running].endtime = t+1;    /* end time */
                    if (trace) printf("\t(process %d terminated)\n", running);
                } else {
                    proc[running].ip++;
                    makeready(running);
                }
                proc[running].runtime++;
                t++;
            }
            
            /*-------------------------------*/
            /* If the process is "computing" */
            /*-------------------------------*/
            else if (a == 'C') {
                n--;            /* reduce remaining computation time */
                
                /* XXX - TO BE WRITTEN */
            }
            
            /*----------------------------------------------------------*/
            /* This point should never be reached if the data is valid. */
            /*----------------------------------------------------------*/
            else {
                fprintf(stderr,"Bad action (%d)\n", a);
                exit(1);
            }
        }
        
        if (!dd) {
            /*----------------*/
            /* If no deadlock */
            /*----------------*/
            printf("All processes successfully terminated.\n");
            for (i=1;i<=np;i++) {
                printf("Process %d: run time = %d, ended at %d\n",
                       i, proc[i].runtime, proc[i].endtime);
            }
        }
        
        putchar('\n');
    }
}

