#include "socket.hh"

#include <cstdlib>
#include <iostream>
#include <span>
#include <string>
#include "tcp_minnow_socket.hh"

using namespace std;

void get_URL( const string& host, const string& path )
{
  CS144TCPSocket sock;
  Address addr = Address( host, "http" );
  sock.connect( addr );
  sock.write( "GET " + path + " HTTP/1.1\r\n" );
  sock.write( "Host: " + host + "\r\n" );
  sock.write( "Connection: close\r\n" );
  sock.write( "\r\n" );

  string buffer;
  string response;

  while ( !sock.eof() ) {
    sock.read( buffer );
    response += buffer;
    buffer.clear();
  }

  cout << response;

  sock.close();
}

int main( int argc, char* argv[] )
{
  try {
    if ( argc <= 0 ) {
      abort(); // For sticklers: don't try to access argv[0] if argc <= 0.
    }

    auto args = span( argv, argc );

    // The program takes two command-line arguments: the hostname and "path" part of the URL.
    // Print the usage message unless there are these two arguments (plus the program name
    // itself, so arg count = 3 in total).
    if ( argc != 3 ) {
      cerr << "Usage: " << args.front() << " HOST PATH\n";
      cerr << "\tExample: " << args.front() << " stanford.edu /class/cs144\n";
      return EXIT_FAILURE;
    }

    // Get the command-line arguments.
    const string host { args[1] };
    const string path { args[2] };

    // Call the student-written function.
    get_URL( host, path );
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
