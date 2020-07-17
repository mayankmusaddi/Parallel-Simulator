# Parallel-Simulator

This is an implementation of a parallel simulation of real life scenarios using pthread. As a part of an assignment, this implementation serves to be an accurate method to learn about parallel processing and the use of pthreads to achieve so.

## Biriyani Service Simulation
The line for serving Biryani is growing day by day in the Kadamb Mess. Annoyed by waiting for hours in the queue we decided to automate the entire pipeline of serving Biryani in the mess. First step in autmoating the process is to first build a simulation and check its pros and cons.

### Implementation
The complete model consists of three entities which are:
 - Robot Chefs
 - Serving Tables
 - Students

##### Robot Chef
Each robot chef is implemented as a thread. As well as a global array *vessels_in_robot* is made which stores the vessels a specific robot has made. Until there are students left the robot continuously generates biriyani. The working of the robot chef can be summed up as follows:

 - **Generate Vessel** - It takes a random time in range (2-5) to generate a random number (1-10) of vessels by incrementing vessels_in_robot by the number of vessels.
 - **biriyani_ready** - It waits until all of its biriyani has been loaded

##### Serving Tables
Each serving table is denoted as a thread. A global array *slots_in_table* is also made which indicates the slots present in a table.
The pipeline for each Serving table is as follows:

 - **Find Vessel** - Iterates through all robots ie *vessels_in_robots* and finds a vessel that can be loaded
 - **Load Vessel** - Decrements vessels made by the robot and generates a random number of slot and calls ready_to_serve_table
 - **ready_to_serve_table** - Waits until any biriyani is left in the serving container.

##### Student
Each student is also implemented as a thread, which follows the steps:

 - **wait_for_slot** - It iterates over all the table ie *slots_in_table* and return a table with an empty slot
 - **student_in_slot** - Decrements slot in the particular table. Waits for the student to consume biriyani (sleep 2)
 - **Student Leaves** - Next the slot is again incremented and global variable *student_left* is decreased indicating the number of students left

Appropriate Mutex are made for each of the robot chefs and tables i.e *load_vessel* and *serve_student*. This would avoid simultaneous vessel selection and the student selection. Apart from that a mutex *students_left_mutex* is made to avoid simultaneous update in the global variable *students_left* which indicated the students still left to be served.

Running Instruction:

```sh
$ gcc biriyani_service_simulation.c -lpthread
$ ./a.out
```

## Ober Cab Service
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