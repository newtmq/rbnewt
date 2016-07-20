# coding: utf-8
Gem::Specification.new do |spec|
  spec.name          = "rbnewt"
  spec.version       = "0.2.0"
  spec.authors       = ["Hiroyasu OHYAMA"]
  spec.email         = ["user.localhost2000@gmail.com"]

  spec.summary       = %q{rbnewt is a fast Ruby client for the STOMP}
  spec.description   = %q{This aims to to be the fastest Ruby client for NewtMQ which is an implementation of STOMP server.}
  spec.homepage      = "https://github.com/userlocalhost2000/rbnewt"
  spec.license       = "MIT"

  spec.extensions    = 'extconf.rb'
  spec.files         = Dir['Makefile', 'src/*.c', 'include/newt/*.h']

  spec.add_runtime_dependency "rspec", "~> 3.4.0"
  spec.add_runtime_dependency "rake", "~> 10.0"
end
