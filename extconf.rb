require "mkmf"

$libs = "-lpthread -lstomp"
$CFLAGS = "-I./include"

def is_header?
  is_ok = true
  is_ok &= have_header('stomp/stomp.h') ||
    find_header('stomp/stomp.h', '/opt/local/include', '/usr/local/include', '/usr/include')
  is_ok &= have_header('pthread.h') ||
    find_header('pthread.h', '/opt/local/include', '/usr/local/include', '/usr/include')
end

def is_library?
  is_ok = true
  is_ok &= have_library('stomp', 'stomp_init') ||
    find_library('stomp', 'stomp_init', '/opt/local/lib', '/usr/local/lib', '/usr/lib')
  is_ok &= have_library('pthread', 'pthread_create') ||
    find_library('pthread', 'pthread_create', '/opt/local/lib', '/usr/local/lib', '/usr/lib')
end

if is_header? and is_library?
  create_makefile('rbnewt', 'src')
else
  raise "Dependent libraries have not been installed yet."
end
