#define _POSIX_C_SOURCE 199309L //required for clock
#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>

struct timespec curtime;

int n,m,k;
int ridersLeft;

pthread_mutex_t cab_mutex[1000000];
pthread_mutex_t rider_mutex[1000000];
pthread_mutex_t ridersLeft_mutex;

typedef struct cab{
    int cabStatus,id1,id2;
}cab;

cab cab_st[1000000];

int riderPaymentStatus[100000]; // 0 - No Transaction, 1 - Waiting, 2 - Accepting

void makePayment(int riderId)
{
    pthread_mutex_lock(&rider_mutex[riderId]);
    printf("Rider %d waiting for payment server to be free\n",riderId);
    riderPaymentStatus[riderId]=1;
    pthread_mutex_unlock(&rider_mutex[riderId]);

    while(riderPaymentStatus[riderId]==1);
}
void EndRide(int cabId,int riderId,int rideTime)
{
    sleep(rideTime);
    printf("Rider %d has reached destination\n",riderId);
    makePayment(riderId);

    pthread_mutex_lock(&cab_mutex[cabId]);
    if(cab_st[cabId].id1 == riderId)
    {
        cab_st[cabId].id1 = -1;
        if(cab_st[cabId].id2==-1)
            cab_st[cabId].cabStatus = 0;
        else
            cab_st[cabId].cabStatus = 2;
    }
    else
    {
        cab_st[cabId].id2 = -1;
        if(cab_st[cabId].id1==-1)
            cab_st[cabId].cabStatus = 0;
        else
            cab_st[cabId].cabStatus = 2;
    }
    printf("Rider %d exits Cab %d\n",riderId,cabId);
    pthread_mutex_unlock(&cab_mutex[cabId]);
}
void AcceptRide(int cabId,int riderId,int cabType, int rideTime)
{
    printf("Rider %d has cab %d assigned with RideTime %d\n",riderId,cabId,rideTime);
    if(cabType==0)
    {
        cab_st[cabId].cabStatus = 1;
        cab_st[cabId].id1 = riderId;
    }
    else
    {
        if(cab_st[cabId].cabStatus==0)
            cab_st[cabId].cabStatus = 2;
        else
            cab_st[cabId].cabStatus = 3;
        
        if(cab_st[cabId].id1==-1)
            cab_st[cabId].id1 = riderId;
        else
            cab_st[cabId].id2 = riderId;
    }
    pthread_mutex_unlock(&cab_mutex[cabId]);

    EndRide(cabId,riderId,rideTime);
}
void BookCab(int cabType,int waitTime,int rideTime,int inTime,int riderId)
{
    char *type = (cabType==0)?"Premier":"Pool";
    printf("Rider %d has booked a cab with type %s\n",riderId,type);
    clock_gettime(CLOCK_MONOTONIC_RAW,&curtime);
    int flag_accepted = 0;
    while(curtime.tv_sec < inTime+waitTime)
    {
        printf("Test : %ld %d %d\n",curtime.tv_sec,inTime,waitTime);
        printf("yo\n");
        for(int i=0;i<n;i++)
        {
            pthread_mutex_lock(&cab_mutex[i]);
            if(cab_st[i].cabStatus==0 || (cab_st[i].cabStatus==2 && cabType==1))
            {
                printf("Done\n");
                flag_accepted = 1;
                AcceptRide(i,riderId,cabType,rideTime);
                return;
            }
            pthread_mutex_unlock(&cab_mutex[i]);
        }
        clock_gettime(CLOCK_MONOTONIC_RAW,&curtime);
    }
    if(!flag_accepted)
    {
        pthread_mutex_lock(&ridersLeft_mutex);
        ridersLeft--;
        printf("Rider %d Timeout\n",riderId);
        printf("Riders Left : %d\n",ridersLeft);
        pthread_mutex_unlock(&ridersLeft_mutex);
    }
}
void *rider(void *tid)
{
    int id = *(int *)tid;
    clock_gettime(CLOCK_MONOTONIC_RAW,&curtime);

    int waitTime = (rand()%2+1);
    int cabType = rand()%2; // 0 - for Premier, 1 - for Pool
    int rideTime = rand()%16+15;

    BookCab(cabType,waitTime,rideTime,curtime.tv_sec,id);
}
void AcceptPayment(int pId)
{
    pthread_mutex_lock(&ridersLeft_mutex);
    while(ridersLeft > 0)
    {
        for(int i=0;i<m;i++)
        {
            pthread_mutex_lock(&rider_mutex[i]);
            if(riderPaymentStatus[i]==1)
            {
                printf("Rider %d has initiated Payment on Server %d\n",i,pId);
                riderPaymentStatus[i]=2;
                pthread_mutex_unlock(&rider_mutex[i]);
                ridersLeft--;
                printf("Riders Left : %d\n",ridersLeft);

                pthread_mutex_unlock(&ridersLeft_mutex);
                sleep(3);
                printf("Rider %d has completed Payment\n",i);
            }
            pthread_mutex_unlock(&rider_mutex[i]);
        }
    }
    pthread_mutex_unlock(&ridersLeft_mutex);
}
void *payment(void *tid)
{
    int id = *(int *)tid;
    AcceptPayment(id);
}

int main()
{
    printf("Enter Number of Cabs : ");
    scanf("%d",&n);
    printf("Enter Number of Riders : ");
    scanf("%d",&m);
    printf("Enter Number of Payment Services : ");
    scanf("%d",&k);

    for(int i=0;i<n;i++)
	    pthread_mutex_init(&cab_mutex[i], NULL);
    for(int i=0;i<m;i++)
	    pthread_mutex_init(&rider_mutex[i], NULL);
    pthread_mutex_init(&ridersLeft_mutex, NULL);

    pthread_t riderid[m];
    pthread_t paymentid[k];

    ridersLeft = m;

    for(int i=0;i<n;i++)
    {
        cab c;
        c.cabStatus = 0; // 0 - waitState, 1 - onRidePremier, 2 - onRidePoolOne, 3 - onRidePoolFull
        c.id1=-1;c.id2=-1;
        cab_st[i] = c;
    }
    for(int i=0;i<k;i++)
    {
        int *id = malloc(sizeof(*id));
        *id=i;
        pthread_create(&paymentid[i], NULL, payment, (void*)(id));
    }
    for(int i=0;i<m;i++)
    {
        int *id = malloc(sizeof(*id));
        *id=i;
        sleep(4);
        pthread_create(&riderid[i], NULL, rider, (void*)(id));
    }

    for(int i=0;i<m;i++)
	    pthread_join(riderid[i], NULL);
    for(int i=0;i<k;i++)
	    pthread_join(paymentid[i], NULL);

    printf("Simulation Complete\n");
    return 0;
}