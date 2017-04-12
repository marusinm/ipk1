BSD Sockets application
===========================================================
## NAME

ftrest - client name
ftrestd - server name

## SYNOPSIS

ftrestd [-r ROOT-FOLDER] [-p PORT]
ftrest COMMAND REMOTE-PATH [LOCAL-PATH]

## DESCRIPTION

ftrestd is server written in C++, it listens for new socket and after connection succed
read and execute rest api command from ftrest client. 

ftrest is client written in C++, it try to connect to ftrestd server with socket, then sends rest api command to preceed. 

ftrest and ftrestd uses http headers with content body.

## HTTP Headers
 
ftrest HTTP request contains these headers:
Date  - clinet timestamp
Accept - type of response
Accept-Encoding - data encoding
Content-Type - MIME type 
Content-Length - Lenght of body

ftrestd respose contains these headers:
Date - server timestapm
Content-Type - MIME type 
Content-Length - Lenght of body
Content-Encoding - data encoding

## RESPONSE CODED
server can response these codes:
200 OK
404 Not Found
400 Bad Reques

## FILES    

All contained files:
ftrest.cpp
ftrestd.cpp
Commander.h - class for system command execution
Parser.h    - class to parse headders
HttpHeader.h -class to compose headers and all contains all info about the header and message
readme.md - markdown

## BUGS AND PRESONAL VIEW

Under pressure of time and fact that this was my first C++ project, the project does not comply with C++ standards on some places. The time was big enamy so there may a few errors ocure at the testing time. Sorry for that. The processor would not be burned down :)

## LINKS

References from which I came:
https://www.tutorialspoint.com/cplusplus/
http://www.restapitutorial.com
https://www.amazon.com/REST-Practice-Hypermedia-Systems-Architecture/dp/0596805829/
https://tools.ietf.org/html/rfc7231
https://guides.github.com/features/mastering-markdown/
http://www.tldp.org/HOWTO/Man-Page/q3.html

