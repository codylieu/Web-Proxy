# Web-Proxy
====
README
Assignment 4
In-Young Jo (ij8) and Cody Lieu (cal53)
Date Started: 4/15/15
Date Completed: 4/22/15
Hours Spent: 25

OBJECTIVE

The goal of this assignment was to understand the HTTP request/forwarding/response process, how proxies work, and caching. In order to accomplish this, a web proxy with caching that stood between a browser and the Internet was designed and implemented in C++.

IMPLEMENTATION

This assignment required us to implement a Web proxy with Caching that lies between the browser and the Internet. Our main function initializes the state variables and then calls the initServer method. initServer sets up the socket between the browser and the proxy and then starts a while loop. This while loop accepts requests from the browser and spawns a thread to handle each response in the processRequest method. The processRequest method takes the request information and extracts the appropriate information (the URL), gets the IP address and port of the web server and passes it to the handleResponse. The handleResponse method first checks to see if the cache contains an entry corresponding to this IP address. If it does, the sendFromCache method is called and then handleResponse returns, cutting out the extra request and response time needed by connecting to the server, increasing throughput. If there is no cache entry a socket is created to connect to the web server. The proxy then forwards the original request to the web server and enters a while loop that continues to forward data from server to client until the server has no other data to send. This indicates the end of the data transfer and then the client and server file descriptors are closed. In this method, nodes are also created for the cache and added to the cache if it’s not already in the cache. Cache implementation levels are below.

Our cache implementation is very efficient, with O(1) get and set time. This is achieved by using a Map and Doubly Linked List combination data structure. The Map and Doubly Linked List are made up of Node structures that hold important information like a vector of charArrays, key, next and prev nodes, and the size in bytes. In handleResponse, if the entry is not in the cache, a new node is created. In the while loop where we are forwarding the response from server to client, we are also adding each response received by recv to the vector of charArrays and adding the corresponding number of bytes received to the node’s size. The addToCache method is then called. This method is where storage limits are enforced. We have a while loop that removes the least recently used node until the cache has enough space to fit the new node. This new node is then added to the map and added to the linked list as the new head and the cache’s size is updated as appropriate. If the entry is in the cache, sendFromCache is called, which iterates through the node’s content array and sends the responses to the client, cutting out the extra latency needed to connect to the server. In both cases, the corresponding socket file descriptors are closed as appropriate. In sendFromCache, we must edit the HTTP header so that the client knows the decoding algorithm to follow.

LIMITING CONDITIONS

We ran into some bugs on our project. The main and only current bug in our project is that we must CTRL-C out of our program for the content that was received by the client to be displayed. Our program was working very robustly previously and didn’t need to do this, but after a merge and subsequent conflicts, this issue appeared and we didn’t know how to fix it.

CONCLUSIONS

Our proxy successfully follows the basic workflow outlined in the project description. The proxy starts from the command line, listens on a certain port (corresponding to an input argument), requests from the browser are forwarded to the proxy and are handled, the proxy creates TCP connections, the proxy spawns threads to process new incoming connections, a least recently used cache is implemented within the proxy, connections are closed when data transferral stops, and the proxy closes when the client closes the connection. Given enough time our proxy is able to load most of the text, images, formatting, etc. Our cache system is functional and LRU is implemented with O(1) get and set for efficiency and increased efficiency for users. Storage limits are enforced in the addToCache method where nodes are evicted until the cache has enough space to store the node, which also holds its size in bytes so that the cache can be decremented by the correct amount when need be. We check for errors and handle them gracefully with the perror library.

After implementing this web proxy, we were able to achieve set at the beginning of the assignment of gaining better understanding of HTTP request/forwarding/response, setting up proxies, and how caching works/can be used in order to improve the efficiency with which web pages are loaded. 



