#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

int m,n,k;
int students_left = 0;

pthread_mutex_t load_vessel[1000000];
pthread_mutex_t serve_student[1000000];
pthread_mutex_t students_left_mutex;

int vessels_in_robot[1000000];
int slots_in_table[1000000];
int biriyani_in_table[1000000];

int wait_for_slot()
{
    while(1)
    {
        for(int i=0;i<n;i++)
        {
            pthread_mutex_lock(&serve_student[i]);
            if(biriyani_in_table[i]>0 && slots_in_table[i]>0)
                return i;
            pthread_mutex_unlock(&serve_student[i]);
        }
    }
}

void student_in_slot(int tid)
{
    slots_in_table[tid]--;
    biriyani_in_table[tid]--;
    pthread_mutex_unlock(&serve_student[tid]);
    sleep(7);
}

void *students(void *tid)
{
    int id = *(int *)tid;
    printf("Student %d has arrived\n",id);
    printf("Student %d is waiting to be allocated a slot on the serving table\n",id);
    int tableid = wait_for_slot();
    printf("Student %d assigned a slot on the serving table %d and waiting to be served\n",id,tableid);
    student_in_slot(tableid);

    pthread_mutex_lock(&students_left_mutex);
    students_left--;
    printf("Student %d on serving table %d has been served.\n",id,tableid);
    printf("Students Left : %d\n",students_left);

    pthread_mutex_lock(&serve_student[tableid]);
    slots_in_table[tableid]++;
    pthread_mutex_unlock(&serve_student[tableid]);

    pthread_mutex_unlock(&students_left_mutex);
    
    // invokes wait_for_slot() which doesn't return until free slot is available
    // i.e. ready_to_serve_table is in progress
    // if student alloted it calls student_in_slot()
}
void biriyani_ready(int id)
{
    printf("Robot Chef %d has prepared %d vessels of Biryani. Waiting for all the vessels to be emptied to resume cooking\n",id,vessels_in_robot[id]);
    while(vessels_in_robot[id]>0)
    {
        if(students_left==0)
            return;
    }
    printf("All the vessels prepared by Robot Chef %d are emptied. Resuming cooking now.\n",id);
}

void *robot_chefs(void *tid)
{
    int id = *(int *)tid;
    printf("Robot Created with ID %d\n",id);

    pthread_mutex_lock(&students_left_mutex);
    while(students_left>0)
    {
        pthread_mutex_unlock(&students_left_mutex);
        int vessels = (rand()%10+1);
        printf("Robot Chef %d is preparing %d vessels of Biryani\n",id,vessels);
        int time = (rand()%4+2);
        sleep(time);

        pthread_mutex_lock(&load_vessel[id]);
        vessels_in_robot[id]+=vessels;
        pthread_mutex_unlock(&load_vessel[id]);

        biriyani_ready(id);
    }
    pthread_mutex_unlock(&students_left_mutex);
    
    // takes w(R(2-5)) seconds to prepare r(R(1-10)) biriyani vessels each having capacity for p(R(25-30)) students
    // if done calls biriyani_ready() which doesn't return until all vessels are loaded
    // once all its vessels are empty, biriyani_ready() function returns and it resumes to step1
}
void ready_to_serve_table(int number_of_slots,int tid)
{
    int servings = (rand()%26+25);
    pthread_mutex_lock(&serve_student[tid]);
    printf("Serving Table %d is ready to serve with %d slots and %d servings\n",tid,number_of_slots,servings);
    biriyani_in_table[tid]+=servings;
    slots_in_table[tid]+=number_of_slots;
    pthread_mutex_unlock(&serve_student[tid]);

    while(biriyani_in_table[tid]>0)
    {
        if(students_left==0)
            return;
    }
}

void *serving_table(void *tid)
{
    int id = *(int *)tid;
    printf("Serving Table Created with ID %d\n",id);

    pthread_mutex_lock(&students_left_mutex);
    while(students_left>0)
    {
        pthread_mutex_unlock(&students_left_mutex);
        for(int i=0;i<m;i++)
        {
            if(vessels_in_robot[i]>0)
            {
                pthread_mutex_lock(&load_vessel[i]);//position will change
                printf("Robot Chef %d is refilling Serving Container of Serving Table %d\n",i,id);
                vessels_in_robot[i]--;
                pthread_mutex_unlock(&load_vessel[i]);

                //serving mode
                printf("Serving table %d entering Serving Phase\n",id);
                int slots = (rand()%10+1);
                ready_to_serve_table(slots,id);
                
                printf("Serving Container of Table %d is empty, waiting for refill\n",id);
            }
        }
    }
    pthread_mutex_unlock(&students_left_mutex);
    
    // loads biriyani vessel
    // creates random number of available slots (x < number of servings left R(1-10))
    // if serving container full -> ready_to_serve_table(int number_of_slots)
    // hangs on above function until
    // - All serving slots full
    // - All waiting students assigned
}
int main()
{
    printf("Enter Number of Robot Chefs : ");
    scanf("%d",&m);
    printf("Enter Number of Serving Tables : ");
    scanf("%d",&n);
    printf("Enter Number of Students : ");
    scanf("%d",&k);

    for(int i=0;i<m;i++)
        pthread_mutex_init(&load_vessel[i], NULL);
    for(int i=0;i<n;i++)
	    pthread_mutex_init(&serve_student[i], NULL);

    pthread_t robotid[m];
    pthread_t tableid[n];
    pthread_t studentid[k];

    students_left = k;

    for(int i=0;i<m;i++)
    {
        int *id = malloc(sizeof(*id));
        *id=i;
        pthread_create(&robotid[i], NULL, robot_chefs, (void*)(id));
    }
    for(int i=0;i<n;i++)
    {
        int *id = malloc(sizeof(*id));
        *id=i;
        pthread_create(&tableid[i], NULL, serving_table, (void*)(id));
    }
    for(int i=0;i<k;i++)
    {
        int *id = malloc(sizeof(*id));
        *id=i;
        sleep(1);
        pthread_create(&studentid[i], NULL, students, (void*)(id));
    }

    for(int i=0;i<m;i++)
	    pthread_join(robotid[i], NULL);
    for(int i=0;i<n;i++)
	    pthread_join(tableid[i], NULL);
    for(int i=0;i<k;i++)
	    pthread_join(studentid[i], NULL);

    printf("Simulation Over\n");
    return 0;
    // stop simulation when all students have been served
    // can use only one mutex lock per Serving Table and Robot Chef
    // take care for dead locks
}