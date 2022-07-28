==================
inerest_api_out_v1
==================
The inerest_api_out_v1 library provides a framework for implementing output
REST API calls in C++ that are compatible with the inerest_api_in_v1 library
available at https://github.com/inesonic/inerest_api_in_v1 as well as other
libraries we've developed internally.

The library is useful for cases where you must interact with cloud based
services from a Qt based C++ application.

The library is currently used by cloud based infrastructure used as part of the
**Aion** low-code algorithm development software and by the now defunct
**SpeedSentry** site monitoring system.  Both products are, or were, supported
and sold by `Inesonic, LLC <https://inesonic.com>`.


Licensing
=========
This library is dual licensed under the MIT license.


Dependencies And Building
=========================
The library is Qt based and is built using either the qmake or cmake build
tool.  You will need to build the library using a recent version of Qt 5.  The
library has also been tested against Qt 6.

The library also depends on the inecrypto library.


qmake
-----
To build inerest_api_in_v1 using qmake:

.. code-block:: bash

   cd inecrypto
   mkdir build
   cd build
   qmake ../inecrypto.pro INECRYPTO_INCLUDE=<path to inecrypto headers>
   make

If you wish to create a debug build, change the qmake line to:

.. code-block:: bash

   qmake ../inecrypto.pro CONFIG+=debug
   make

Note that the qmake build environment currently does not have an install target
defined and will alway build the library as a static library.


cmake
-----
To build inerest_api_out_v1 using cmake:

.. code-block:: bash

   cd inerest_api_out_v1
   mkdir build
   cmake -B. -H.. -DCMAKE_INSTALL_PREFIX=/usr/local/
   make

To install, simply run

.. code-block:: bash

   make install

You can optionally also include any of the following variables on the cmake
command line.

+-------------------------+---------------------------------------------------+
| Variable                | Function                                          |
+=========================+===================================================+
| inerest_api_out_v1_TYPE | Set to ``SHARED`` or ``STATIC`` to specify the    |
|                         | type of library to be built.   A static library   |
|                         | will be built by default.                         |
+-------------------------+---------------------------------------------------+
| INECRYPTO_INCLUDE       | You can set this variable to indicate the         |
|                         | location of the inecrypto header files.  This     |
|                         | variable only needs to be set on Windows or if    |
|                         | the headers are in a non-standard location.       |
+-------------------------+---------------------------------------------------+
| INECRYPTO_LIB           | You can set this variable to indicate the full    |
|                         | path to the inecrypto static or shared library.   |
|                         | This variable is only needed on Windows, if the   |
|                         | library is in a non-standard location, or if      |
|                         | cmake can-not locate the library after setting    |
|                         | the ``INECRYPTO_LIBDIR`` variable.                |
+-------------------------+---------------------------------------------------+
| INECRYPTO_LIBDIR        | You can use this variable to add one or more      |
|                         | directories to the inecrypto library search path. |
|                         | Separate paths with spaces.                       |
+-------------------------+---------------------------------------------------+


Using The Library In Your Code
==============================
The entire inerest_api_out_v1 library API is contained within the
``RestApiOutV1`` namespace.

To use, you'll need to instantiate an instance of ``RestApiOutV1::Server`` that
tracks information specific to a server you will interact with.  You can tie
multiple outbound REST API classes to a single server which can then share
information.

Once you define a server, you can define one or more outbound REST API
endpoints using the classes:

* ``RestApiOutV1::InesonicRestHandler``
* ``RestApiOutV1::InesonicBinaryRestHandler``

You can either overload these classes using the ``RestApiOut::*::process*``
to intercept the responses, or you can tie the signals in the REST API endpoint
handlers to slots in your code to handle the responses.

You issue requests using the ``RestApiOut::*::post`` methods.

The classes will handle the entire process of sending out the requests.


Inesonic REST API Message Format
================================
For details on the supported message format, please see the documentation for
the `inerest_api_in_v1 <https::github.com/inesonic/inerest_api_in_v1>` library.
