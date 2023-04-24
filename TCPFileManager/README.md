# TCP File Server in JAVA
A special purpose file server where students (clients) will be able to upload, download and request for files. JAVA programming language was used for the implementation.

### _Core Functionalities_
1. Each student has a specific IP address. A student can Login (i.e., connect to the server) with his/her student ID. If the student ID is already logged in from another IP address, new login is denied by the server and the connection is immediately terminated with proper message. So, each student is mapped to a single IP address at any instance. When a student gets connected for the first time, the server creates a directory with the student ID as directory name. All the files uploaded by this student will be stored in this directory.

2. While connected, a student can:
a. Look up the list of all the students who were connected at least once to the server. The students who are currently online should be distinguishable in the list.
b. Look up the list of all the files (both private and public) uploaded by him/her and download any of these files. Private and public files should be distinguishable in the list.
c. Look up the public files of a specific student and download any of these files.
d. Request for a file. This request (short file description and request id) will be broadcasted to all the connected students.
e. View all the unread messages.
f. Upload a file.
i. The file can be either public or private and it will be specified by the uploader.
ii. If the file is uploaded in response to a request, it must be public and the uploader will provide valid request Id.
g. When a requested file has been uploaded, the server sends a message to the person who requested for that file. Multiple students can upload the same requested file and the server will send as many messages.

### _Supported Commands_


- Print user list: pul
- View my files: ls
- View other's files: ls -o user_id



- Upload file: up file_name access_param
- Upload file against a request: upreq file_name requestID



- Download my file: down access_param file_name
- Download other's file: down -o user_id file_name
- Download using fileID: down fileID



- Display Messages: dm


### _Specifications_
For detailed implementation specifications, check out the [Specification](https://github.com/ksaifahmed/networking_ns3_CSE322/blob/main/TCPFileManager/Specifications.pdf)