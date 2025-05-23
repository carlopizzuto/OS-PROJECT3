[05/07] [17:45]
I have just finished reading the instructions and am starting to plan how I will implement the project. I think the most important thing to do at the beggining is to create the basic structure / skeleton of the project and when everything is working I will start to add more features, so I'll start by creating a simple program that can read the input file and print the data to the console and move on from there.

[05/08] [1:00]
I have just finished creating the basic structure of the project and I am now starting to implement the functions. I started with the `create` command and fully implemented it. I also started on the `insert` command but I haven't finished it yet. I'm also starting the process of restructuring my project by splitting the b-tree and io functions from the main file, as well as the constants and other helper functions.

[05/08] [13:00]
Today I plan on finishing the `insert` command, which implies that I need to work on the b-tree implementation. I'll start by creating the b-tree structure and then I will implement the functions for the other commands.

[05/08] [14:25]
I have just finished the restructuring of the project. I created constants and utils files and modified the Makefile to handle the new structure and it still works. Later on today I will continue working on the `insert` command and hopefully finish a basic b-tree implementation by tonight.

[05/08] [22:30]
I plan on having a quick dev session and see if I can finish the `insert` command and start on the `search` command. 

[05/09] [02:00]
I have just finished the `insert` command. The B-tree is working as expected, splitting and inserting nodes like a B-tree should. The only thing missing is to fill the 'ghost' data (after split) with 0's, as it can be confusing to debug. Tomorrow I'll start with the `search` command with I don't think will take long, and then move on to the next ones.

[05/10] [22:50]
Today I plan on having a quick dev session to format / comment the code and see if there are any logical errors. If I have time I'll also start the search command.

[05/11] [1:00]
I was adding some comments and removing bad code when I came across a bug in the split mechanism, the parent id of the original root was not being updated in the index file, so I fixed it. I also made some changes to the `create` command to make it more understandable structure wise. I may do some other small changes, but right now the b-tree is working as expected and tomorrow I'll start on the `search` command.

[05/11] [17:00]
I plan on finishing everything today. I'll probably start by implementing the search command and then I'll move on to the `load`, `print` and `extract` commands.

[05/11] [22:15]
I've just finished implementing all commands. There's still some formatting to do, but I'm happy with the final structure of the code. I'll submit the project now, but may still do some formatting later on. Overall, I'm happy with how the project turned out, I learned a lot about b-trees and file I/O in C and had a lot of fun working on the code.