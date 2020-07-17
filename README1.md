# Biriyani Service Simulation
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