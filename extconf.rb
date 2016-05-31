require "mkmf"

def is_header?
  have_header('stomp/stomp.h') || find_header('stomp/stomp.h', '/opt/local/include', '/usr/local/include', '/usr/include')
end

def is_library?
  have_library('stomp', 'stomp_session_new') ||
    find_library('stomp', 'stomp_session_new', '/opt/local/lib', '/usr/local/lib', '/usr/lib')
end

if is_header? and is_library?
  create_makefile("rbnewt")
else
  raise "libstomp has not been installed yet."
end
