Melon
=====

Melon: Converter that produces PDF from CNKI proprietary formats

Development
-----------

Currently, CAJ and KDH can be converted. Please report
any failures with a sample that can reproduce the behaviour.

HN support is being worked on.

Usage
=====

`make`

Optionally, `make install`

`melon -o OUTPUT INPUT`

Options
-------

-o, --output  
Specify output file  

-b, --buffer  
Set buffer size (default 512k)  

-v, --verbose  
Print more information (twice for even more)

Thanks
======

This project is inspired by [https://github.com/JeziL/caj2pdf](https://github.com/JeziL/caj2pdf)
