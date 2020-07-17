# Ober Cab Service
Ober is a new cab service whose working system is simulated. The requirements of the system are as follows given N cabs, M riders and K payment servers a working system is to be designed which ensures correctness and idempotency. Each cab has one driver. In Ober, payments by the riders should be done in Payment Servers only. Ober provides two types of cab services namely pool and premier ride. In pool ride maximum of two riders can sharea cab whereas in premier ride only one rider.

### Implementation
The complete model consists of three entities which are:
 - Driver
 - Rider
 - Payment Server

##### Driver
Each driver is constructed as a struct *cab* which stores the Cab Status and the ids of the rider. We maintain an array of these structs *cab_st* to denote all the drivers which are there.

The Cab Status indicates the Mode in which the cab is presently in. There are namely 4 modes:
 - **waitSite** (0) - Cab is empty and waiting for passengers
 - **onRidePremier** (1) - Cab has a passenger and will not take more
 - **onRidePoolOne** (2) - Cab has 1 passenger and can take 1 more passenger
 - **onRidePoolFull** (3) - Cab has 2 passengers and its capacity is full

At the time of initialization all the cabs are made of cab status 0 with ids of passenger -1 denoting no passenger

##### Rider
Each rider is denoted as a thread. With each rider is associated its *waitTime* (the time it would wait for a cab booking), *cabType* (the cab it would take Premier or Pool), *rideTime* (the time it will take on the ride). Apart from this an array *riderPaymentStatus* is made which indicates the payment status of the rider (0 for No transaction, 1 for waiting for transaction, 2 for transaction being accepted).
The Pipeline that each Rider is as follows:
 - **BookCab** - Here if the rider's waitTime has not been exceeded then it would iterate over all the cab struct and search for compatible empty cabs. If such a cab is available it goes to AcceptRide or else it goes to *TimeOut*
 - **AcceptRide** - Accept ride changes the status of the cab appropriately and calls the EndRide function
 - **EndRide** - It waits until the rideTime of the rider and then calls the MakePayment which initiates a payment with the server. After the payment is initiated it again changes the status og the cab appropriately
 - **MakePayment** - Changes the payment status of the rider to waiting for transaction i.e. *riderPaymentStatus* is updated to 1. Then is waits until is transaction is getting accepted i.e. *riderPaymentStatus* turns to 2

##### Payment Server
Each payment server is also implemented as a thread, which calls the *AcceptPayment* function.
 - **AcceptPayment** - If riders are left it would iterate over their *riderPaymentStatus* array and constantly search if any rider is waiting for transaction. After getting such rider it changes the payment status to accepting and then initiates the payment indicated by waiting for 3 seconds.

Appropriate Mutex are made for each of the cabs and riders i.e *cab_mutex* and *rider_mutex*. This would avoid simultaneous cab selection and the payment server selection. Apart from that a mutex *ridersLeft_mutex* is made to avoid simultaneous update in the global variable *ridersLeft* which indicated the riders still left to be served.

Running Instruction:

```sh
$ gcc ober_cab_service.c -lpthread
$ ./a.out
```