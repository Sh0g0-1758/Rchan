add_library (stream_copy STATIC bidirectional_stream_copy.cc)

# First find OpenSSL
find_package(OpenSSL REQUIRED)

macro(add_app exec_name)
  add_executable("${exec_name}" "${exec_name}.cc")
  target_link_libraries("${exec_name}" 
    PRIVATE
    stream_copy
    minnow_debug
    util_debug
  )

  if(${exec_name} STREQUAL "server")
    target_link_libraries("${exec_name}" 
      PRIVATE 
      OpenSSL::SSL 
      OpenSSL::Crypto
    )
  endif()
endmacro(add_app)

add_app(webget)
add_app(client)
add_app(server)