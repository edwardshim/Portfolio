This file should contain:

  - your name
  - your UNI
  - lab assignment number
  - description for each part
  
The description should indicate whether your solution for the part is
working or not.  You may also want to include anything else you would
like to communicate to the grader such as extra functionalities you
implemented or how you tried to fix your non-working code.

Name: Eddie Shim
UNI: ess2222
Lab #7

Part 1 is working:

Please go to clac.cs.columbia.edu/~ess2222/tng/index.html for my website.

Part 2a is working and code is commented inside http-server.c

1. Type "make" and press ENTER
2. Start ./mdb-lookup-server mdb-cs3157 <localhostport>
3. Type "./http-server <port> ~/html localhost <localhostport>

In mdb-lookup-server, you will see "connection started from <myip>".

4. In a browser, please type the 3 test cases:

clac.cs.columbia.edu:<port>/cs3157/tng/index.html
clac.cs.columbia.edu:<port>/cs3157/tng/
clac.cs.columbia.edu:<port>/cs3157/tng 

For the third case, only the index.html will pop-up (as clarified in the
listserv).  

You may also CTRL+C from netcat while connected to the http-server.  It will
not crash.

Part 2b is working and code is commented inside http-server.c

1. Type "make"
2. Start ./mdb-lookup-server mdb-cs3157 <localhostport>
3. Type "./http-server <port> ~/html localhost <localhostport>
4. In a browser, please type:

clac.cs.columbia.edu:<port>/mdb-lookup (or mdb-lookup?= or
mdb-lookup?key=<searchterm> to test for other cases)

Your query will return a table as specified and will be ready for another
query.  Simply pressing "ENTER" will return all queries.
