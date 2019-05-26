# C++ HTTP Server

In this repository you'll find an HTTP server made with C++ (using Boost/Beast). The server has been made to be generic about the requests handler.

# Image Writer

In addition, you'll see a `main.cpp` with an example of how to use this server and give it a request handler. The `imgwriter` receives an image and writes a given text over it (with OpenCV).

Note: `test-client.py` is a Python script that would help you to test the server.

The `imgwriter` server expects this JSON structure:

Param. | Descripción
--- | --- 
`text` | Text to be written
`x_pos` | X position (pixel) where text will be placed
`y_pos` | Y position (pixel) where text will be placed
`image` | Original image, where text will be written

If you want to see how to use the server please checkout `main.cpp` and the simple `Makefile` version that is also available in this repository.



