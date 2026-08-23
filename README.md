
# http-server

Yet another http server...


This repository is a clone of the github repostiory 

 https://github.com/infraredCoding/cerveur

which was made for fun and I did some minor performance related changes


## Here's what changed.

- Uses `binary tree` this one uses `hashtables` for routing (stolen).

- Uses chunk-by-chunk streaming instead of loading whole files at once (which older version did.)

- Added generation of  dynamic headers in place (instead of just sending `HTTP/1.0 200 OK`). 

- Added support for diffrent status code rather than just `200`

- Added headers `Content-type` and `Content-Length` which older one lacked.

- Added support for diffrent mimetypes.


## Warning

This is just an experimental server. Don't use it in production

( I know no-one is such a big fool;

but there are infinite possiblities, 

so this warning is for  such fools. )


## Credits

The credits goes to the respective owners which the author may not own and 

the author disclaims any and every kind of copyright to the files

- src/hash\_table.c

- include/hash\_table.h

